

//定义区
#define configUSE_TIMERS  1
#define SSD1306_128_64
//依赖导入区
#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoUZlib.h>

#include <SPIFFS.h>

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "AsyncJson.h"
#include "ArduinoJson.h"
#include <NTPClient.h>
#include <sys/time.h>
#include<time.h>
#include <Wire.h>
#include <WebSocketsClient.h>

#include <HTTPClient.h>

#include <HardwareSerial.h>
//引入蓝牙键盘鼠标库
#include <BleKeyboard.h>
//结构体
struct WeatherInfo {
    String code;
    String tmp;
    String humidity;
    String text;
    String locationData;
    String windDir;
    String windScale;
};
WeatherInfo wa;
//常量配置区
const String wifiPath = "/wifi.txt";
const String weatherConfigPath = "/weather.txt";
//函数定义区

void vApplicationStackOverflowHook(TaskHandle_t xTask ,
    signed char* pcTaskName);

void createConnectWiFiTask();
void createAPTask();
void createServerTask();
void createWiFiStateTask();
void createTimeUpdateTask();
void createGetWeatherTask();
void createXiaoXiaoMengSerialTask();
void createWsClientTask();
void createBLEComboTask();
void vTaskTimeUpdate(void* p);
void vTaskConnectWifi(void* param);
void vTaskCreateWiFiAP(void* p);
void vTaskCreateServer(void* param);
void vTaskCreateWiFiStateListener(void* p);
void vTaskScreenInitial(void* params);
void vTaskCreateWeatherInfo(void* p);
void vTaskCreateWsClientTask(void* p);
void vTaskCreateXiaoXiaoMengSerialListener(void* p);
void vTaskCreateBLEComboTask(void* p);
void webConfigureWiFi(AsyncWebServerRequest* request , JsonVariant& json);
void configureWeather(AsyncWebServerRequest* request , JsonVariant& json);
void onEvent(WStype_t type , uint8_t* payload , size_t length);
String getWifiConfig();
String* getWeatherConfig();
void saveWeatherConfig(String content);
void initWeather(String key , String location);
void commandHandler(String instructions);
void saveWiFiConfig(String content);
