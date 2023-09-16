#include <main.h>
//变量定义区
TaskHandle_t connectWifiTask = NULL;
TaskHandle_t createWiFiAP = NULL;
TaskHandle_t createServer = NULL;
TaskHandle_t createWiFiListener = NULL;
TaskHandle_t createTimeUpdate = NULL;
TaskHandle_t createBLEScaner = NULL;
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP , "ntp.ntsc.ac.cn"); // NTP客户端
BLEScan* pBLEScan;
class BLEDeviceCallBack : public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {

        if (advertisedDevice.haveName()) {
            Serial.printf("发现设备: %s\n" , advertisedDevice.getName().c_str());

        }

    }
};


void setup() {
    Serial.begin(115200);
    Serial.flush();
    Serial.flush();
    Serial.flush();
    Serial.println("Wellcome");
    if (!SPIFFS.begin()) {
        Serial.println("打开LittleFs失败");
        return;
    }
    //继电器引脚
    pinMode(16 , OUTPUT);

    WiFi.mode(WIFI_MODE_APSTA);
    Serial.println("创建任务中...");
    createConnectWiFiTask();
    createAPTask();
    createServerTask();
}
//创建WiFi状态监听任务
void createWiFiStateTask() {
    if (xTaskCreate(
        vTaskCreateWiFiStateListener ,
        "createWiFiStateListener" ,
        5120 ,
        NULL ,
        1 ,
        &createWiFiListener
    ) != pdPASS) {
        Serial.println("创建WiFi状态监听器失败");
        vTaskDelete(createWiFiListener);
    }

}
//创建WiFi监听
void vTaskCreateWiFiStateListener(void* p) {
    while (1) {
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("WiFi连接已断开,尝试重连中");
            createConnectWiFiTask();
            vTaskDelete(NULL);
            return;
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}
//创建Web任务
void createServerTask() {
    if (xTaskCreate(
        vTaskCreateServer ,
        "createServer" ,
        5120 ,
        NULL ,
        1 ,
        &createServer
    ) != pdPASS) {
        vTaskDelete(createServer);
        Serial.println("创建web服务器任务失败");

    }
    Serial.printf("createServerTask查看实际的堆栈使用量%d\n" , uxTaskGetStackHighWaterMark(createServer));

}
//创建热点任务
void createAPTask() {
    if (xTaskCreate(
        vTaskCreateWiFiAP ,
        "createAP" ,
        2048 ,
        NULL ,
        1 ,
        &createWiFiAP
    ) != pdPASS) {
        vTaskDelete(createWiFiAP);
        Serial.println("创建WiFi热点失败");
    }
    Serial.printf("createAPTask查看实际的堆栈使用量%d\n" , uxTaskGetStackHighWaterMark(createWiFiAP));

}
//创建一个连接WiFi任务
void createConnectWiFiTask() {
    if (xTaskCreate(
        vTaskConnectWifi ,
        "ConnectWifi" ,
        2048 ,
        NULL ,
        1 ,
        &connectWifiTask
    ) != pdPASS) {
        vTaskDelete(connectWifiTask);
        Serial.println("创建连接WiFi任务失败");
    }
    Serial.printf("createConnectWiFiTask查看实际的堆栈使用量%d\n" , uxTaskGetStackHighWaterMark(connectWifiTask));

}
//创建时间更新任务
void createTimeUpdateTask() {
    if (xTaskCreate(
        vTaskTimeUpdate ,
        "timeUpdate" ,
        2048 ,
        NULL ,
        1 ,
        &createTimeUpdate
    ) != pdPASS) {
        Serial.println("创建时间更新服务失败");
        vTaskDelete(createTimeUpdate);
    }
    Serial.printf("createTimeUpdateTask查看实际的堆栈使用量%d\n" , uxTaskGetStackHighWaterMark(createTimeUpdate));

}
void vTaskTimeUpdate(void* p) {
    timeClient.setRandomPort();
    timeClient.begin();
    timeClient.setTimeOffset(28800);
    pinMode(16 , OUTPUT);
    while (1) {
        timeClient.update();
        vTaskDelay(1000 / portTICK_PERIOD_MS);

    }
    vTaskDelete(NULL);
}
//创建web服务器，包含Ws
void vTaskCreateServer(void* param) {
    Serial.println("正在创建web服务器");

    AsyncCallbackJsonWebHandler* wifiConfigureHandler = new AsyncCallbackJsonWebHandler("/wifi/submit" , webConfigureWiFi);
    AsyncCallbackJsonWebHandler* lockHandler = new AsyncCallbackJsonWebHandler("/bike/lock" , lockBike);

    ws.onEvent(onEvent);
    AsyncStaticWebHandler* staticServer = new AsyncStaticWebHandler("/" , SPIFFS , "/web" , "none");
    staticServer->setDefaultFile("index.html").setCacheControl("max-age=6000");
    server.addHandler(staticServer);
    server.addHandler(lockHandler);
    server.addHandler(wifiConfigureHandler);
    server.addHandler(&ws);
    server.begin();
    vTaskDelete(NULL);
}

//连接WiFi任务
void vTaskConnectWifi(void* param) {
    String* content = getWifiConfig();
    if (content == NULL) {
        createAPTask();
        vTaskDelete(NULL);
        return;
    }
    Serial.println(*content);
    DynamicJsonDocument config(200);
    deserializeJson(config , *content);
    String ssid = (config) ["ssid"].as<String>();
    String pwd = (config) ["pwd"].as<String>();

    Serial.printf("正在连接到%s" , ssid);
    while (WiFi.status() != WL_CONNECTED) {
        WiFi.begin(ssid , pwd);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }


    Serial.printf("已连接到%s\n" , ssid);
    Serial.flush();
    createWiFiStateTask();
    createTimeUpdateTask();
    createBLEScanTask();


    vTaskDelete(NULL);
}


//Ws事件处理
void onEvent(AsyncWebSocket* server , AsyncWebSocketClient* client , AwsEventType type ,
    void* arg , uint8_t* data , size_t len) {
    switch (type) {
    case WS_CONNECTED:
    Serial.printf("客户端%s:%u已连接\n" , client->remoteIP().toString() , client->remotePort());
    break;

    default:
    break;
    }
}
//建立AP
void vTaskCreateWiFiAP(void* p) {

    IPAddress localIp = IPAddress(192 , 168 , 1 , 1);
    IPAddress gateWay = IPAddress(192 , 168 , 1 , 1);
    IPAddress subNet = IPAddress(255 , 255 , 255 , 0);
    WiFi.softAPConfig(localIp , gateWay , subNet);
    WiFi.softAP("wusui_Ya" , "Qinsansui233..." , 12 , 0 , 16);
    vTaskDelete(NULL);
}


//WiFi配置接口
//POST ssid,pwd
void webConfigureWiFi(AsyncWebServerRequest* request , JsonVariant& json) {
    DynamicJsonDocument jsonObj = json.as<JsonObject>();
    String content = jsonObj.as<String>();
    if (!content.isEmpty()) {
        request->send(200 , "application/json" ,
            F("{\"code\":0,\"msg\":\"配置完成\"}"));
        saveWiFiConfig(content);

    } else {
        request->send(200 , "application/json" ,
            F("{\"code\":-1,\"msg\":\"配置信息不完全\"}"));
    }


}
void lockBike(AsyncWebServerRequest* request , JsonVariant& json) {
    DynamicJsonDocument jsonObj = json.as<JsonObject>();
    String pwd = jsonObj ["pass"].as<String>();
    if (pwd != "Qinsansui233...") {
        request->send(404);
        return;
    }
    bool needLock = jsonObj ["needLock"].as<bool>();
    if (needLock) {
        digitalWrite(16 , HIGH);
    } else {
        digitalWrite(16 , LOW);

    }

    request->send(200);

}
//获取WiFi配置
String* getWifiConfig() {
    File f = SPIFFS.open(wifiPath , "r");
    if (!f) {
        Serial.println("获取WiFi配置失败，将启动热点并开启服务器进行配网操作");
        f.close();
        return NULL;
    }
    String* content = new String();
    *content = f.readString();
    f.close();
    if (content->isEmpty()) {
        Serial.println("WiFi配置为空");
        return NULL;
    }
    return content;
}

//保存WiFi配置
void saveWiFiConfig(String content) {
    File f = SPIFFS.open(wifiPath , "w+" , true);
    if (!f) {
        Serial.println("不能创建文件");
        return;
    }
    f.print(content);
    f.flush();
    f.close();

    createConnectWiFiTask();
}
//创建蓝牙扫描任务
void createBLEScanTask() {
    if (createBLEScaner != NULL) {
        Serial.println("已经创建了蓝牙扫描任务");
        return;
    }

    if (xTaskCreate(
        vTaskBLEScaner ,
        "BLEScaner" ,
        5120 ,
        NULL ,
        0 ,
        &createBLEScaner
    ) != pdPASS) {
        vTaskDelete(createBLEScaner);
        Serial.println("创建蓝牙扫描任务失败");
    }

}
//蓝牙扫描任务
void vTaskBLEScaner(void* p) {
    Serial.println("开启蓝牙扫描");
    BLEDevice::init("ESP32");
    pBLEScan = BLEDevice::getScan();
    pBLEScan->setActiveScan(true);
    pBLEScan->setInterval(0x50);
    pBLEScan->setWindow(0x10);

    int i = 0;
    while (1) {
        i++;
        bool hasWatch = false;
        Serial.printf("执行一次扫描%d\n" , i);
        taskYIELD();
        BLEScanResults found = pBLEScan->start(5);
        taskYIELD();
        int count = found.getCount();
        for (int i = 0; i < count; i++) {
            BLEAdvertisedDevice device = found.getDevice(i);
            std::string name = device.getName().c_str();
            std::string deviceMac = device.getAddress().toString().c_str();
            std::string useName = "Mi_Watch";
            std::string mac = "70:58:96:08:a6:4e";
            if (deviceMac == mac) {
                Serial.println("有小米手表");
                hasWatch = true;
                break;
            } else {
                Serial.printf("Device:%s %d\n" , device.getAddress().toString().c_str() , device.getRSSI());
            }
        }
        if (hasWatch) {
            digitalWrite(16 , HIGH);
        } else {
            digitalWrite(16 , LOW);
        }

        Serial.printf("一次扫描结束%d\n" , i);
        vTaskDelay(1000 / portTICK_PERIOD_MS);


    }
    Serial.println("蓝牙扫描结束");
    vTaskDelete(NULL);

}
void loop() {
    vTaskDelete(NULL);
}