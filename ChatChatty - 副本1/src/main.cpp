/**
 * @file main.cpp
 * @author 宁子希 (1589326497@qq.com)
 * @brief ChatChatty AI聊天机器人
 * @version 1.0
 * @date 2024-06-09
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include <Arduino.h>
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
#include "base64.h"
#include "cJSON.h"
#include "Audio.h"
// #include <TFT_eSPI.h>
#include <Arduino_GFX_Library.h>
// #include "esp_core_dump.h"


/*
 *  
 * 版本号     v1.0
 *                                    
 */ 
#define VERSION "v1.0"


#define VOLUME 20 // 音量范围: 0...21 (0-63db)

#define SPI_FREQUENCY  40000000   // 设置 SPI 频率为 40MHz，可以根据需要调整此值

//-------------------引脚定义-------------------------
// 定义按键和max9814麦克风引脚
#define KEY_PIN 3
#define ADC_PIN 2
// 定义max98357扬声器引脚
#define I2S_DOUT 6  // DIN连接
#define I2S_BCLK 5  // 位时钟
#define I2S_LRC 4   // 左右时钟


// 定义显示屏驱动
#define GC9A01_DRIVER
// ESP32-S3 TFT屏引脚设置
#define TFT_MISO -1  // 没有使用 MISO，引脚设为 -1
#define TFT_SCLK 18  // SPI 时钟引脚
#define TFT_MOSI 17  // SPI MOSI 引脚
#define TFT_RST  16  // 复位引脚
#define TFT_DC   15  // 数据/命令控制引脚
#define TFT_CS   14  // 芯片选择控制引脚
#define TFT_BL   13  // 背光控制引脚

// 定义LED端口
const int LED_A_PIN = 10;  //LED端口
const int LED_B_PIN = 11;  //LED端口

//-------------定义录音数据和JSON数据的长度-------------------
hw_timer_t *timer = NULL;
const int adc_data_len = 16000 * 3;
const int data_json_len = adc_data_len * 2 * 1.4;
uint16_t* adc_data;
// String data_json;
char * data_json;
uint8_t adc_start_flag = 0;
uint8_t adc_complete_flag = 0;


//--------------------AP网络参数定义--------------------
const char* APssid = "ChatChatty"; // AP的名称
const char* APpassword = "123456789"; // AP的密码
IPAddress local_IP(192, 168, 4, 1); // 静态IP地址
IPAddress gateway(192, 168, 4, 1); // 网关IP地址
const char* deviceName = "ChatChatty"; // 设置设备的名称



// ----------------------NVS 键名-------------------------------
const char* nvs_namespace = "config";
// // Wi-Fi 网络配置
const char* key_stassid = "STAssid";
const char* key_stapassword = "STApassword";
// minimax 配置
const char* key_minikey = "Mini_api_key";
const char* key_ttsid = "Mini_ID";

// 百度 API 鉴权配置
const char* key_clientid = "Baidu_TTS_ID";
const char* key_clientsecret = "Baidu_TTS_key";

//音色
const char* key_voice_id = "voice_id";



// ----------------发送MiniMax的信息-------------
String inputText = "你好，ChatChatty！";

String response, question, aduiourl;  //存储响应信息，存储用户输入的问题，存储URL
String answer = "我是宁子希的小助手，你好鸭";
DynamicJsonDocument jsonDoc(1024);
uint32_t num = 0;
uint32_t time1, time2;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED; //共享资源

// ----------------------函数声明-------------------------------
String getAccessToken( String _Baidu_TTS_ID, String _Baidu_TTS_key);
bool connectToWiFi();
void performOTAUpdate();
void startConfigServer();
void setupToneConfig();
void serverTask(void *pvParameters);
void screenTask(void *parameter);
// void displayGIF(File file);


void IRAM_ATTR onTimer();
String sendToSTT();
String getGPTAnswer(String inputText);
String getvAnswer(String ouputText);

//实例对象
WebServer server(80);
Preferences preferences;  //实例化Preferences 对象

//音频处理对象 和 http请求对象
Audio audio;
//HTTPClient http, http1, http2;
HTTPClient http;

// TFT
// 创建数据总线和显示对象
Arduino_DataBus *bus = new Arduino_ESP32SPI(TFT_DC, TFT_CS, TFT_SCLK, TFT_MOSI, TFT_MISO, HSPI);
Arduino_GFX *gfx = new Arduino_GC9A01(bus, TFT_RST, 0 /* rotation */, true /* IPS */);


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
    <p>ChatChatty，是一款基于 ESP32 的 AI 聊天平台，利用 MiniMax AI 大模型，带来富有情感韵律的语音体验。我们致力于让聊天更生动、更愉快、更智能。欢迎来体验，感受 AI 聊天的独特魅力！</p>
    <button onclick="window.location='/update'">固件更新</button>
    <button onclick="window.location='/config'">配置页面</button>
    <button onclick="window.location='/tone'">音色配置</button>
    <p style="font-size: 0.8em; color: #888;">
        版本 )rawliteral" VERSION R"rawliteral( 本项目由宁子希创建并维护，遵循 GPLv3 开源协议。未经作者明确许可，不得用于商业用途。所有权利保留。
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
        
    
        <label for="Mini_ID">Mini ID：</label>
        <input type="text" name="Mini_ID" id="Mini_ID" placeholder="请输入 Mini ID">
    
        <label for="Mini_api_key">Mini API 密钥：</label>
        <input type="text" name="Mini_api_key" id="Mini_api_key" placeholder="请输入 Mini API 密钥">
    
        
        <label for="Baidu_TTS_ID">百度TTS ID：</label>
        <input type="text" name="Baidu_TTS_ID" id="Baidu_TTS_ID" placeholder="请输入客户端 ID">
        
        <label for="Baidu_TTS_key">百度TTS 密钥：</label>
        <input type="text" name="Baidu_TTS_key" id="Baidu_TTS_key" placeholder="请输入客户端密钥">
        
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



//--------------------UTL信息-------------------------
// 定义语音模型ID和API密钥
String voice_id; //音色配置

// 百度语音识别

const char* stt_url = "http://vop.baidu.com/server_api";
String accessToken;

// MiniMax Chat API配置
const char* chat_url = "https://api.minimax.chat/v1/text/chatcompletion_v2";
const char* base_url = "https://api.minimax.chat/v1/t2a_pro?GroupId=";
String Mini_api_key;
String Mini_ID;

String Mini_Token_key;




void setup() {
  // esp_core_dump_init(); //初始化核心转存
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
    1                    
  );

  // 等待Wi-Fi连接完成
  pinMode(LED_B_PIN, OUTPUT); // 设置LED引脚模式
  pinMode(LED_A_PIN, OUTPUT);
  digitalWrite(LED_B_PIN,LOW);
  digitalWrite(LED_A_PIN,LOW);

  // 尝试连接到 Wi-Fi
  connectToWiFi();  //LED_B和LED_A同时闪烁: nvs中已加载到配网信息

  digitalWrite(LED_B_PIN,LOW);
  digitalWrite(LED_A_PIN,LOW);
  Serial.println("请在配置界面配置Wi-Fi");  //只有LED_B闪烁：nvs中没有配网信息，等待在控制台配置
  while (WiFi.status() != WL_CONNECTED) { 
    delay(500);
    digitalWrite(LED_B_PIN,!digitalRead(LED_B_PIN));  //LEDB闪烁
    Serial.print(".");
  }

  digitalWrite(LED_B_PIN,HIGH); //LED_B常亮:网络已经成功连接

  //-------------------------------从nvs获取信息-------------------------------------------
  preferences.begin(nvs_namespace, true);

  //MiniMax API密钥，MiniMax id
  Mini_api_key =preferences.getString(key_minikey,"");
  Mini_ID=preferences.getString(key_ttsid,"");
  Mini_Token_key = String("Bearer ") + Mini_api_key;

  //获取访问令牌
  
  String Baidu_TTS_ID = preferences.getString(key_clientid, "");
  String Baidu_TTS_key = preferences.getString(key_clientsecret, "");
  preferences.end();

  Serial.println("nvs获取信息完成");

  accessToken = getAccessToken(Baidu_TTS_ID,Baidu_TTS_key); //本地鉴权，获取令牌

  if (accessToken != "null") {
    Serial.print("访问令牌: ");
    Serial.println(accessToken);
  } else {
    Serial.println("获取访问令牌失败");
  }

  // 从NVS读取voice_id
  preferences.begin(nvs_namespace, true);
  voice_id = preferences.getString(key_voice_id, "female-shaonv");
  preferences.end();
  Serial.print("当前语音id: ");
  Serial.println(voice_id);

  //测试
  Serial.print("Mini ID: ");
  Serial.println(Mini_ID);
  Serial.print("Mini API 密钥: ");
  Serial.println(Mini_api_key);
  Serial.print("Mini Token 密钥: ");
  Serial.println(Mini_Token_key);
  Serial.print("百度TTS ID: ");
  Serial.println(Baidu_TTS_ID);
  Serial.print("百度TTS 密钥: ");
  Serial.println(Baidu_TTS_key);
  

  
  //程序逻辑 版本1.0
  Serial.print("ChatChatty  版本:");
  Serial.println(VERSION);

  
  adc_data = (uint16_t *)ps_malloc(adc_data_len * sizeof(uint16_t));  //ps_malloc 指使用片外PSRAM内存
  if (!adc_data) {  // 分配内存失败
    Serial.println("无法为 adc_data 分配内存");
  }
  data_json = (char *)ps_malloc(data_json_len * sizeof(char));  // 根据需要调整大小
  if (!data_json) { // 分配内存失败
    Serial.println("无法为data_json分配内存");
  }

  pinMode(ADC_PIN, ANALOG);
  pinMode(KEY_PIN, INPUT_PULLUP);

  //音频数据I2S总线的引脚配置
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  // 设置音频的播放音量
  audio.setVolume(VOLUME);
  
  http.setTimeout(5000); //响应超时时间
 
  //设置定时器16k采样音频
  timer = timerBegin(0, 40, true);    //  80M的时钟 40分频 2M
  timerAlarmWrite(timer, 125, true);  //  2M  计125个数进中断  16K
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmEnable(timer);
  timerStop(timer);  //先暂停


  delay(2000);
  //----------------------spi_TFT----------------------
  // 创建屏幕显示任务
  xTaskCreatePinnedToCore(
    screenTask,     // 任务函数
    "ScreenTask",   // 任务名称
    1024*10,           // 任务堆栈大小（字节）
    NULL,           // 传递给任务函数的参数
    1,              // 任务优先级
    NULL,           // 任务句柄
    1               // CPU 核心编号
  );
  

  digitalWrite(LED_A_PIN,HIGH); //数据初始化与web初始化全部完成PIN_A常亮

}

void loop() {
  // server.handleClient();

  audio.loop();
  if (digitalRead(KEY_PIN) == 1) {
    delay(10);
    if (digitalRead(KEY_PIN) == 1) {
      Serial.printf("开始识别\r\n");
      digitalWrite(LED_A_PIN, LOW);
      adc_start_flag = 1; //设置ADC（模数转换器）开始采集的标志
      timerStart(timer);
      while (!adc_complete_flag)  //等待采集完成
      {
        ets_delay_us(10);
      }
      timerStop(timer);
      adc_complete_flag = 0;  //清标志
      digitalWrite(LED_A_PIN, HIGH);  //完成采集打开LED灯

      //发送音频数据到语音识别引擎，获取识别结果
      question = sendToSTT(); 
      if (question != "error") {
        Serial.println("输入:" + question);

        //获取GPT的回答
        answer = getGPTAnswer(question);  
        if (answer != "error") {
          Serial.println("回答: " + answer);
          aduiourl = getvAnswer(answer);
          if (aduiourl != "error") {
            audio.stopSong(); //停止播放。
            audio.connecttohost(aduiourl.c_str());  //  连接到回答URL，并播放音频 128k mp3
          }
        }
      }
      Serial.println("识别完成\r\n");
    }
  }
  vTaskDelay(5);
}

//--------------处理来自客户端的 HTTP 请求----------------------
void serverTask(void *pvParameters) {
  for (;;) {
    server.handleClient(); // 处理客户端请求
    // MDNS.update(); // 更新mDNS服务
    vTaskDelay(5); // 留出时间给其他任务执行
  }
}


//----------------------百度TTS本地鉴权---------------------------------
String getAccessToken( String _Baidu_TTS_ID, String _Baidu_TTS_key) {

  // 动态生成获取访问令牌的 URL
  String token_url = "https://aip.baidubce.com/oauth/2.0/token?grant_type=client_credentials&client_id=" + _Baidu_TTS_ID + "&client_secret=" + _Baidu_TTS_key;
  Serial.println("----------------百度TTS本地鉴权开始------------");
  // 创建 HTTP 客户端
  HTTPClient http;
  http.begin(token_url);

  int httpResponseCode = http.POST("");
  String accessToken = "";

  if (httpResponseCode > 0) {
    String response = http.getString();
    // Serial.println("访问令牌 JSON:");
    // Serial.println(response);


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
      digitalWrite(LED_B_PIN,!digitalRead(LED_B_PIN));  //LEDB闪烁
      digitalWrite(LED_A_PIN,!digitalRead(LED_A_PIN));  //LEDB闪烁
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
    String Mini_api_key = server.arg("Mini_api_key");
    String Mini_ID = server.arg("Mini_ID");
    String Baidu_TTS_ID = server.arg("Baidu_TTS_ID");
    String Baidu_TTS_key = server.arg("Baidu_TTS_key");

    // 保存到 NVS
    Serial.println("正在保存信息到NVS");
    Serial.println("空余nvs空间: ");
    Serial.print(preferences.freeEntries());//获取空余nvs空间
    Serial.println();

    preferences.begin(nvs_namespace, false);
    preferences.putString(key_stassid, stassid);
    preferences.putString(key_stapassword, stapassword);
    preferences.putString(key_minikey, Mini_api_key);
    preferences.putString(key_ttsid, Mini_ID);
    preferences.putString(key_clientid, Baidu_TTS_ID);
    preferences.putString(key_clientsecret, Baidu_TTS_key);
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
      String _voice_id = server.arg("voice_id");

      // 保存到 NVS
      preferences.begin(nvs_namespace, false);
      preferences.putString(key_voice_id, _voice_id);
      preferences.end();

      server.sendHeader("Content-Type", "text/html; charset=utf-8");
      server.send(200, "text/plain", "<span style='font-size: 36px;'> 音色配置已保存。 </span>");
      Serial.print("新的音色: ");
      Serial.println(_voice_id);

      //将当前音色修改
      voice_id=_voice_id;
    } else {
      server.sendHeader("Content-Type", "text/html; charset=utf-8");
      server.send(400, "text/plain", "<span style='font-size: 36px;'> 缺少配置参数 </span>");
    }
  });
}

//----------------------------------录音函数--------------------------------------------
void IRAM_ATTR onTimer() {
//增加计数器并设置 ISR 时间
  portENTER_CRITICAL_ISR(&timerMux);
  if (adc_start_flag == 1) {  //检查是否正在录制音频
    
    adc_data[num] = analogRead(ADC_PIN);
    num++;
    if (num >= adc_data_len) {  //数据收集足够
      adc_complete_flag = 1;
      adc_start_flag = 0;
      num = 0;
    }
  }
  portEXIT_CRITICAL_ISR(&timerMux);
}



// -----------------------------------发送数据到语音转文字（STT）服务-------------------------------
//stt语音识别
String sendToSTT() {
  //添加JSON格式的开头部分，包括音频格式、采样率、设备ID、通道数、用户ID和访问令牌等参数。
  memset(data_json, '\0', data_json_len * sizeof(char));
  strcat(data_json, "{");
  strcat(data_json, "\"format\":\"pcm\",");
  strcat(data_json, "\"rate\":16000,");
  strcat(data_json, "\"dev_pid\":1537,");
  strcat(data_json, "\"channel\":1,");
  strcat(data_json, "\"cuid\":\"57722200\",");
  strcat(data_json, "\"token\":\"");
  strcat(data_json, accessToken.c_str());
  strcat(data_json, "\",");
  sprintf(data_json + strlen(data_json), "\"len\":%d,", adc_data_len * 2);
  strcat(data_json, "\"speech\":\"");
  strcat(data_json, base64::encode((uint8_t *)adc_data, adc_data_len * sizeof(uint16_t)).c_str());  //将音频数据进行Base64编码
  strcat(data_json, "\"");
  strcat(data_json, "}");

  
  http.begin(stt_url);  //请求的URL https://vop.baidu.com/pro_api
  http.addHeader("Content-Type", "application/json");
  
  // 发送POST请求
  int httpResponseCode = http.POST(data_json);

  if (httpResponseCode == 200) {
    response = http.getString();
    http.end();

    // 打印回应json
    //Serial.print(response);

    deserializeJson(jsonDoc, response);
    String question = jsonDoc["result"][0];
    // 访问"result"数组，并获取其第一个元素

    return question;
  } else {

    http.end();
    Serial.printf("stt_error: %s\n", http.errorToString(httpResponseCode).c_str());
    return "error";
  }
}


//-----------------------------------------------获取回答------------------------------------------------
//minichat对话获取回答和chatGPT方式相同
String getGPTAnswer(String inputText) {
  
  HTTPClient http1;
  http1.begin(chat_url);
  http1.addHeader("Content-Type", "application/json");
  http1.addHeader("Authorization", Mini_Token_key);

  // 构建 JSON 负载
  String payload = "{\"model\":\"abab5.5s-chat\",\"messages\":[{\"role\": \"system\",\"content\": \"你叫ChatChatty是宁子希的生活助手机器人，要求下面的回答严格控制在256字符以内。\"},{\"role\": \"user\",\"content\": \"" + inputText + "\"}]}";

  // 发送 HTTP POST 请求
  int httpResponseCode = http1.POST(payload);

  if (httpResponseCode == 200) {  // 如果响应成功
    response = http1.getString();
    http1.end();

    //打印响应内容
    //Serial.println(response);

    //清除之前的 JSON 文档数据
    jsonDoc.clear();

    // 反序列化 JSON 响应
    DeserializationError error = deserializeJson(jsonDoc, response);
    if (error) {  // 检查反序列化是否成功
      Serial.printf("deserializeJson() 失败: %s\n", error.c_str());
      return "error";
    }

    // 获取回答内容
    String answer = jsonDoc["choices"][0]["message"]["content"];
    if (answer) {
      return answer;
    } else {
      Serial.println("获取回答过程 JSON 响应中没有结果");
      return "error";
    }
  } else {  // 如果响应失败
    String response = http1.getString();  // 获取响应内容
    http1.end();
    Serial.printf("chatError %i \n", httpResponseCode);
    Serial.println(response);  // 打印错误响应
    return "error";
  }
}


//-----------------------------tts语音播报------------------------------------
String getvAnswer(String ouputText) {
  HTTPClient http2;  // 每次调用时创建新的 HTTPClient 实例

  String tts_url = String(base_url) + String(Mini_ID);
  http2.begin(tts_url);
  http2.addHeader("Content-Type", "application/json");
  http2.addHeader("Authorization", Mini_Token_key);

  // 创建 JSON 对象并填充数据
  StaticJsonDocument<200> doc;
  doc["text"] = ouputText;
  doc["model"] = "speech-01";
  doc["audio_sample_rate"] = 32000;
  doc["bitrate"] = 128000;
  doc["voice_id"] = voice_id;

  // 序列化 JSON 对象到字符串
  String jsonString;
  serializeJson(doc, jsonString);

  // 发送 HTTP POST 请求
  int httpResponseCode = http2.POST(jsonString);

  if (httpResponseCode == 200) {  // 如果响应成功
    String response = http2.getString();
    http2.end();

    // 清除之前的 JSON 文档数据
    jsonDoc.clear();

    // 反序列化 JSON 响应
    DeserializationError error = deserializeJson(jsonDoc, response);
    if (error) {  // 检查反序列化是否成功
      Serial.printf("deserializeJson() 失败: %s\n", error.c_str());
      return "error";
    }

    // 获取音频文件 URL
    const char* audioUrl = jsonDoc["audio_file"];
    if (audioUrl) {
      return String(audioUrl);
    } else {
      Serial.println("JSON 响应中没有音频文件 URL");
      return "error";
    }
  } else {  // 如果响应失败
    Serial.printf("tts %i \n", httpResponseCode);
    String response = http2.getString();  // 获取响应内容
    http2.end();
    Serial.println(response);  // 打印错误响应
    return "error";
  }
}


// 屏幕显示任务
void screenTask(void *parameter) {

  // 初始化 TFT 屏幕
  gfx->begin();
  gfx->fillScreen(BLACK);


  // 背光控制
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);

    // 在屏幕上显示文本
  gfx->setCursor(50, 50);
  gfx->setTextColor(RED);
  gfx->println("Hello World!");

  // vTaskDelete(NULL);
  while (true) {
    delay(1000); 
  }
}

