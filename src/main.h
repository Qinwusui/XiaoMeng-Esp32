

//定义区
#define configUSE_TIMERS  1

//依赖导入区
#include <Arduino.h>
#include <WiFi.h>

#include <SPIFFS.h>

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "AsyncJson.h"
#include "ArduinoJson.h"
#include <NTPClient.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
//常量配置区
const String wifiPath = "/wifi.txt";

//对象定义区


//函数定义区
void createConnectWiFiTask();
void createAPTask();
void createServerTask();
void createWiFiStateTask();
void createTimeUpdateTask();
void createBLEScanTask();

void vTaskBLEScaner(void* p);
void vTaskTimeUpdate(void* p);
void vTaskConnectWifi(void* param);
void vTaskCreateWiFiAP(void* p);
void vTaskCreateServer(void* param);
void vTaskCreateWiFiStateListener(void* p);

void lockBike(AsyncWebServerRequest* request , JsonVariant& json);
void webConfigureWiFi(AsyncWebServerRequest* request , JsonVariant& json);
void onEvent(AsyncWebSocket* server , AsyncWebSocketClient* client , AwsEventType type ,
    void* arg , uint8_t* data , size_t len);
void saveWiFiConfig(String content);
String* getWifiConfig();


