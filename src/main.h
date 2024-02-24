

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
#include <img.h>
#include <sys/time.h>
#include<time.h>
#include <Wire.h>

// #include <SSD1306Wire.h>
// #include <OLEDDisplayUi.h>
#include <HTTPClient.h>

#include <U8g2lib.h>
#include <HardwareSerial.h>

//结构体
struct WeatherInfo {
    String code;
    String tmp;
    String humidity;
    String text;
    String locationData;
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
void createScreenInitialTask();
void createGetWeatherTask();
void createReadGPSTask();
//多个屏幕绘制的方法

void draw();

void vTaskTimeUpdate(void* p);
void vTaskConnectWifi(void* param);
void vTaskCreateWiFiAP(void* p);
void vTaskCreateServer(void* param);
void vTaskCreateWiFiStateListener(void* p);
void vTaskScreenInitial(void* params);
void vTaskCreateWeatherInfo(void* p);
void vTaskReadGPS(void* params);
void lockBike(AsyncWebServerRequest* request , JsonVariant& json);
void webConfigureWiFi(AsyncWebServerRequest* request , JsonVariant& json);
void configureWeather(AsyncWebServerRequest* request , JsonVariant& json);
void onEvent(AsyncWebSocket* server , AsyncWebSocketClient* client , AwsEventType type ,
    void* arg , uint8_t* data , size_t len);
void saveWiFiConfig(String content);
String* getWifiConfig();
String* getWeatherConfig();
void saveWeatherConfig(String content);
void initWeather(String key , String location);
