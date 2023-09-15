//依赖导入区
#include <Arduino.h>
#include <WiFi.h>

#include <LittleFS.h>

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

//常量配置区
const String wifiPath = "wifi.conf";

//函数定义区
void vTaskConnectWifi(void* param);

//对象定义区
struct WifiConfig {
    String SSID;
    String PWD;
};
WifiConfig* getWifiConfig();

void vTaskCreateWiFiAP(void* p);
void webConfigureWiFi(AsyncWebServerRequest* request);