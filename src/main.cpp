#include <main.h>
//变量定义区
TaskHandle_t connectWifiTask = NULL;
TaskHandle_t createWiFiAP = NULL;
AsyncWebServer server(80);
void setup() {
    Serial.begin(115200);
    Serial.flush();
    Serial.flush();
    Serial.flush();
    Serial.println("WelCome");
    if (!LittleFS.begin(true)) {
        Serial.println("打开SPIFFS失败");
        return;
    }
    server.serveStatic("/" , LittleFS , "/web/").setDefaultFile("index.html");
    server.on("/wifi/submit" , HTTP_POST , webConfigureWiFi);
    server.begin();
    Serial.println("创建任务中...");
    if (xTaskCreate(
        vTaskConnectWifi ,
        "ConnectWifi" ,
        10240 ,
        NULL ,
        10 ,
        &connectWifiTask
    ) != pdPASS) {
        vTaskDelete(connectWifiTask);
        Serial.println("创建连接WiFi任务失败");
    }

    if (xTaskCreate(
        vTaskCreateWiFiAP ,
        "createWiFiServer" ,
        10240 ,
        NULL ,
        1 ,
        &createWiFiAP
    ) != pdPASS) {
        vTaskDelete(createWiFiAP);
        Serial.println("创建web服务器任务失败");
    }

}

void loop() {
    vTaskDelete(NULL);
}

void vTaskConnectWifi(void* param) {
    WifiConfig* config = getWifiConfig();
    if (config == NULL) {
        xTaskCreate(
            vTaskCreateWiFiAP ,
            "createWiFiServer" ,
            10240 ,
            NULL ,
            1 ,
            &createWiFiAP
        );
        vTaskDelete(connectWifiTask);
    }
    WiFi.begin(config->SSID , config->PWD);
    Serial.printf("正在连接到%s" , config->SSID);
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    Serial.println();
    Serial.printf("已连接到%s\n" , config->SSID);
    Serial.flush();
    vTaskDelete(connectWifiTask);
}
WifiConfig* getWifiConfig() {
    File f = LittleFS.open(wifiPath , "r");
    if (!f) {
        Serial.println("获取WiFi配置失败，将启动热点并开启服务器进行配网操作");
        f.close();
        return NULL;
    }
    String content = f.readString();

    WifiConfig* config = new WifiConfig();
    int index = content.indexOf("\\||");
    String ssid = content.substring(0 , index);
    String pwd = content.substring(index + 1 , content.length());
    Serial.printf("获取WiFi配置成功%s:%s\n" , ssid , pwd);
    config->SSID = ssid;
    config->PWD = pwd;
    return config;
}
void vTaskCreateWiFiAP(void* p) {
    WiFi.mode(WIFI_MODE_APSTA);

    IPAddress localIp = IPAddress(192 , 168 , 1 , 1);
    IPAddress gateWay = IPAddress(192 , 168 , 1 , 1);
    IPAddress subNet = IPAddress(255 , 255 , 255 , 0);
    WiFi.softAPConfig(localIp , gateWay , subNet);
    WiFi.setSleep(false);
    WiFi.softAP("wusui_Ya" , "Qinsansui233..." , 12 , 0 , 16);
    vTaskDelete(NULL);
}
//WiFi配置接口
//POST ssid,pwd
void webConfigureWiFi(AsyncWebServerRequest* request) {
    if (request->hasParam("ssid" , true)) {
        Serial.println(request->getParam("ssid" , true)->value());
    }
    if (request->hasParam("pwd" , true)) {
        Serial.println(request->getParam("pwd" , true)->value());
    }

    request->send(200);
}

