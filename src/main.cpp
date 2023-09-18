#include <main.h>
//变量定义区
TaskHandle_t connectWifiTask = NULL;
TaskHandle_t createWiFiAP = NULL;
TaskHandle_t createServer = NULL;
TaskHandle_t createWiFiListener = NULL;
TaskHandle_t createTimeUpdate = NULL;
TaskHandle_t createWiFiScanner = NULL;
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP , "ntp.ntsc.ac.cn"); // NTP客户端



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
    WiFi.scanNetworks(true);
    
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
        20480 ,
        NULL ,
        1 ,
        &createServer
    ) != pdPASS) {
        vTaskDelete(createServer);
        Serial.println("创建web服务器任务失败");

    }

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
    //主页
    server.on("/" , [] (AsyncWebServerRequest* request) {
        request->send(SPIFFS , "/web/index.html" , "text/html");
        });
    //WiFi扫描接口
    server.on("/wifi/startScan" , HTTP_GET , [] (AsyncWebServerRequest* request) {

        Serial.println("开启WiFi扫描");
        DynamicJsonDocument* json = new DynamicJsonDocument(80 * 30 + 20);
        int i = WiFi.scanComplete();
        if (i > 0) {
            for (int j = 0; j < i; j++) {
                DynamicJsonDocument* item = new DynamicJsonDocument(80);

                std::string  ssid = WiFi.SSID(j).c_str();
                int rssi = WiFi.RSSI(j);
                std::string enc = (WiFi.encryptionType(j) == WIFI_AUTH_OPEN)?"开放": "加密";
                (*item) ["name"] = ssid;
                (*item) ["strength"] = rssi;
                (*item) ["encryption"] = enc;
                Serial.printf("WiFi:%s\n" , ssid.c_str());
                (*json) ["list"][j] = (*item);
            }
        }
        if (i == -1) {
            Serial.println("扫描正在进行中");
        }
        if (i == -2) {
            Serial.println("扫描失败");
        }

        WiFi.scanNetworks(true);
        Serial.println("WiFi扫描结束");
        std::string* jsonStr = new  std::string();
        serializeJson(*json , *jsonStr);
        AsyncWebServerResponse* response = request->beginResponse(200 , "application/json" , (*jsonStr).c_str());
        response->addHeader("Access-Control-Allow-Credentials" , "true");
        response->addHeader("Access-Control-Allow-Methods" , "*");
        response->addHeader("Access-Control-Allow-Origin" , "http://127.0.0.1");
        request->send(response);
        free(json);
        free(jsonStr);
        });
    //WiFi状态接口
    server.on("/wifi/status" , HTTP_GET , [] (AsyncWebServerRequest* request) {
        DynamicJsonDocument* json = new DynamicJsonDocument(50);
        (*json) ["code"] = 0;
        (*json) ["status"] = WiFi.status();
        std::string* jsonStr = new std::string();
        deserializeJson(*json , *jsonStr);
        AsyncWebServerResponse* response = request->beginResponse(200 , "application/json" , (*jsonStr).c_str());
        response->addHeader("Access-Control-Allow-Credentials" , "true");
        response->addHeader("Access-Control-Allow-Methods" , "*");
        response->addHeader("Access-Control-Allow-Origin" , "http://127.0.0.1");
        request->send(response);
        free(json);
        free(jsonStr);
        });
    //axios文件获取接口
    server.on("/axios.min.js.gz" , HTTP_GET , [] (AsyncWebServerRequest* request) {
        AsyncWebServerResponse* response = request->beginResponse(SPIFFS , "/web/axios.min.js.gz" , "text/javascript");
        response->addHeader("Content-Encoding" , "gzip");
        request->send(response);
        });
    //vue.js文件获取接口
    server.on("/vue.global.prod.min.js.gz" , HTTP_GET , [] (AsyncWebServerRequest* request) {
        AsyncWebServerResponse* response = request->beginResponse(SPIFFS , "/web/vue.global.prod.min.js.gz" , "text/javascript");
        response->addHeader("Content-Encoding" , "gzip");
        request->send(response);
        });
    //elementUI.js文件获取接口
    server.on("/index.full.min.js.gz" , HTTP_GET , [] (AsyncWebServerRequest* request) {
        AsyncWebServerResponse* response = request->beginResponse(SPIFFS , "/web/index.full.min.js.gz" , "text/javascript");
        response->addHeader("Content-Encoding" , "gzip");
        request->send(response);
        });
    //elementUI.css文件获取接口
    server.on("/index.min.css.gz" , HTTP_GET , [] (AsyncWebServerRequest* request) {
        AsyncWebServerResponse* response = request->beginResponse(SPIFFS , "/web/index.min.css.gz" , "text/javascript");
        response->addHeader("Content-Encoding" , "gzip");
        request->send(response);
        });
    //main.js
    server.on("/main.js.gz" , HTTP_GET , [] (AsyncWebServerRequest* request) {
        AsyncWebServerResponse* response = request->beginResponse(SPIFFS , "/web/main.js.gz" , "text/javascript");
        response->addHeader("Content-Encoding" , "gzip");
        request->send(response);
        });
    server.on("/favicon.ico" , HTTP_GET , [] (AsyncWebServerRequest* request) {
        AsyncWebServerResponse* response = request->beginResponse(SPIFFS , "/web/favicon.ico" , "image/*");
        response->addHeader("Content-Encoding" , "gzip");
        request->send(response);
        });
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
    String ssid = (config) ["name"].as<String>();
    String pwd = (config) ["pwd"].as<String>();
    Serial.printf("正在连接到%s" , ssid);
    //连接之前扫描一遍附近的所有WiFi，当WiFi存在时才进行连接操作，否则直接跳过连接
    vTaskDelay(4000/portTICK_PERIOD_MS);

    int i = WiFi.scanComplete();
    for (int j = 0; j < i; j++) {
        Serial.println(WiFi.SSID(j));

        if (WiFi.SSID(j) != ssid) {
            continue;
        } else {
            goto hasSSID;
            return;
        }
    }
    goto notHasSSID;
hasSSID:
    {
        WiFi.begin(ssid , pwd);
        while (WiFi.status() != WL_CONNECTED) {

            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
        Serial.printf("已连接到%s：IP:%s\n" , ssid , WiFi.localIP().toString().c_str());
        Serial.flush();
        createWiFiStateTask();
        createTimeUpdateTask();
        vTaskDelete(NULL);

    }
notHasSSID:
    {
        Serial.println("附近无该WiFi，将结束该任务");
        WiFi.disconnect();
        vTaskDelete(NULL);

    }
    Serial.println("www");
    WiFi.scanNetworks(true);
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
    WiFi.softAP("wusui_Ya" , "Qinsansui233..." , 12 , 0 , 1);
    vTaskDelete(NULL);
}


//WiFi配置接口
//POST ssid,pwd
void webConfigureWiFi(AsyncWebServerRequest* request , JsonVariant& json) {
    DynamicJsonDocument jsonObj = json.as<JsonObject>();
    String content = jsonObj.as<String>();
    DynamicJsonDocument* respJson = new DynamicJsonDocument(50);
    if (!content.isEmpty()) {
        (*respJson) ["code"] = 0;
        (*respJson) ["msg"] = "配置完成，正在连接" + jsonObj ["name"].as<String>();
        saveWiFiConfig(content);
    } else {
        (*respJson) ["code"] = -1;
        (*respJson) ["msg"] = "配置信息缺失";
    }
    String* respStr = new String();
    deserializeJson(*respJson , *respStr);
    request->send(200 , "application/json" , *respStr);

    free(respJson);
    free(respStr);

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
void loop() {
    vTaskDelete(NULL);
}