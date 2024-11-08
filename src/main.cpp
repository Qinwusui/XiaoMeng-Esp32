#include <main.h>
//变量定义区
TaskHandle_t connectWifiHandler = NULL;
TaskHandle_t createWiFiAPHandler = NULL;
TaskHandle_t createServerHandler = NULL;
TaskHandle_t createWiFiListenerHandler = NULL;
TaskHandle_t createTimeUpdateHandler = NULL;
TaskHandle_t createWiFiScannerHandler = NULL;
TaskHandle_t initScreenHandler = NULL;
TaskHandle_t initWeatherHandler = NULL;
TaskHandle_t readGPSHandler = NULL;
TaskHandle_t readXiaoXiaoMengSerialHandler = NULL;
TaskHandle_t createWsClientHandler = NULL;
TaskHandle_t createBLEComboHandler = NULL;
AsyncWebServer server(80);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP , "ntp.ntsc.ac.cn"); // NTP客户端
HardwareSerial xxm(2);
WebSocketsClient wsClient;
BleKeyboard keyBoard;
void setup() {
    Serial.begin(115200);

    Serial.flush();
    Serial.flush();
    Serial.flush();
    Serial.println("Wellcome");
    if (!SPIFFS.begin()) {
        Serial.println("打开Fs失败");
        return;
    }

    WiFi.mode(WIFI_MODE_APSTA);
    WiFi.scanNetworks(true);

    Serial.println("Initialized...");

    createConnectWiFiTask();
    createAPTask();
    createBLEComboTask();

    //暂不需要
    // createServerTask();
    // createXiaoXiaoMengSerialTask();

}
void createBLEComboTask() {
    if (xTaskCreate(
        vTaskCreateBLEComboTask ,
        "vTaskCreateBLEComboTask" ,
        4096 ,
        NULL ,
        1 ,
        &createBLEComboHandler
    ) != pdPASS) {
        Serial.println("创建蓝牙键鼠任务失败");
    }

}
void vTaskCreateBLEComboTask(void* p) {
    keyBoard.setName("五岁的键盘");
    keyBoard.begin();
    Serial.println("蓝牙键盘已开启");

    vTaskDelete(NULL);
}
void createWsClientTask() {
    if (xTaskCreate(
        vTaskCreateWsClientTask ,
        "vTaskCreateWsClientTask" ,
        5120 ,
        NULL ,
        1 ,
        &createWsClientHandler
    ) != pdPASS) {
        Serial.println("创建小小萌串口监听失败");
    }
}
void vTaskCreateWsClientTask(void* p) {
    wsClient.begin("xiaoxiaomeng.liusui.xyz" , 8080 , "/ws");
    wsClient.enableHeartbeat(1000 , 2000 , 50);
    wsClient.setAuthorization("wusui" , "Qinsansui233...");
    wsClient.setReconnectInterval(2000);
    String s = "Device: " + String((char*) ESP.getChipModel());
    wsClient.setExtraHeaders(s.c_str());

    wsClient.onEvent(onEvent);
    Serial.println("已连接Ws");
    while (1) {
        wsClient.loop();
    }
    vTaskDelete(NULL);
}
//小小萌串口监听
void createXiaoXiaoMengSerialTask() {
    if (xTaskCreate(
        vTaskCreateXiaoXiaoMengSerialListener ,
        "createXiaoXiaoMengSerialTask" ,
        20480 ,
        NULL ,
        1 ,
        &readXiaoXiaoMengSerialHandler
    ) != pdPASS) {
        Serial.println("创建小小萌串口监听失败");
    }

}
//小小萌串口监听
void vTaskCreateXiaoXiaoMengSerialListener(void* p) {
    Serial.println("启动小小萌串口监听");
    xxm.begin(9600 , SERIAL_8N1 , 16 , 17);
    while (1) {
        if (xxm.available() > 0) {
            String   s = xxm.readStringUntil('\n');
            s.trim();
            commandHandler(s);
        }
    }
    vTaskDelete(NULL);
}
//串口指令处理
void commandHandler(String instructions) {
    // if (instructions == "openLight") {
    //     xxm.print("light_Opened");
    // }
    // if (instructions == "closeLight") {
    //     xxm.print("light_Closed");
    // }
    // if (instructions == "openConditioner") {
    //     xxm.print("conditioner_Opened");
    // }
    // if (instructions == "closeConditioner") {
    //     xxm.print("conditioner_Closed");
    // }
    // Serial.println(instructions);
    // xxm.flush();
    pinMode(12 , OUTPUT);

    if (instructions == "openLight") {
        digitalWrite(12 , HIGH);
        wsClient.sendTXT("isNoReply:light_Opened");
    }
    if (instructions == "closeLight") {
        wsClient.sendTXT("isNoReply:light_Closed");
        digitalWrite(12 , LOW);
    }
    if (instructions == "lock_PC") {
        if (keyBoard.isConnected()) {
            keyBoard.press(KEY_LEFT_GUI);
            keyBoard.press('l');
            delay(100);
            keyBoard.releaseAll();
            Serial.println("锁定电脑");
            wsClient.sendTXT("isNoReply:pc_Locked");
        }

    }
    if (instructions == "unlock_PC") {
        if (keyBoard.isConnected()) {
            keyBoard.press(KEY_BACKSPACE);
            vTaskDelay(1500 / portTICK_PERIOD_MS);
            keyBoard.print("Qinsansui233...");
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            keyBoard.press(KEY_RETURN);
            keyBoard.releaseAll();
            Serial.println("解锁电脑");
            wsClient.sendTXT("isNoReply:pc_Unlocked");

        }

    }




}
//创建WiFi状态监听任务
void createWiFiStateTask() {
    if (xTaskCreate(
        vTaskCreateWiFiStateListener ,
        "createWiFiStateListener" ,
        2048 ,
        NULL ,
        1 ,
        &createWiFiListenerHandler
    ) != pdPASS) {
        Serial.println("创建WiFi状态监听器失败");
        vTaskDelete(createWiFiListenerHandler);
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
        40960 ,

        NULL ,
        1 ,
        &createServerHandler
    ) != pdPASS) {
        vTaskDelete(createServerHandler);
        Serial.println("创建web服务器任务失败");

    }
    Serial.printf("createServerTask查看实际的堆栈使用量%d\n" , uxTaskGetStackHighWaterMark(createServerHandler));


}
//创建热点任务
void createAPTask() {
    if (xTaskCreate(
        vTaskCreateWiFiAP ,
        "createAP" ,
        2048 ,
        NULL ,
        1 ,
        &createWiFiAPHandler
    ) != pdPASS) {
        vTaskDelete(createWiFiAPHandler);
        Serial.println("创建WiFi热点失败");
    }
    Serial.printf("createAPTask查看实际的堆栈使用量%d\n" , uxTaskGetStackHighWaterMark(createWiFiAPHandler));

}
//创建一个连接WiFi任务
void createConnectWiFiTask() {
    if (xTaskCreate(
        vTaskConnectWifi ,
        "ConnectWifi" ,
        2048 ,
        NULL ,
        1 ,
        &connectWifiHandler
    ) != pdPASS) {
        vTaskDelete(connectWifiHandler);
        Serial.println("创建连接WiFi任务失败");
    }
    Serial.printf("createConnectWiFiTask查看实际的堆栈使用量%d\n" , uxTaskGetStackHighWaterMark(connectWifiHandler));

}
//创建时间更新任务
void createTimeUpdateTask() {
    if (xTaskCreate(
        vTaskTimeUpdate ,
        "timeUpdate" ,
        2048 ,
        NULL ,
        1 ,
        &createTimeUpdateHandler
    ) != pdPASS) {
        Serial.println("创建时间更新服务失败");
        vTaskDelete(createTimeUpdateHandler);
    }
    Serial.printf("createTimeUpdateTask查看实际的堆栈使用量%d\n" , uxTaskGetStackHighWaterMark(createTimeUpdateHandler));

}
void vTaskTimeUpdate(void* p) {
    // timeClient.setRandomPort();
    // timeClient.setRandomPort();
    timeClient.begin();
    timeClient.setTimeOffset(28800);


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
    AsyncCallbackJsonWebHandler* weatcherHandler = new AsyncCallbackJsonWebHandler("/weather/submit" , configureWeather);

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
                (*item) ["ssid"] = ssid;
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
        vPortFree(json);
        vPortFree(jsonStr);
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
        delete json;
        delete jsonStr;
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
    server.addHandler(wifiConfigureHandler);
    server.addHandler(weatcherHandler);
    server.begin();

    vTaskDelete(NULL);
}

//连接WiFi任务
void vTaskConnectWifi(void* param) {
    String content = getWifiConfig();
    if (content == NULL) {
        createAPTask();
        vTaskDelete(NULL);
        return;
    }
    Serial.println(content);
    DynamicJsonDocument config(200);
    deserializeJson(config , content);
    String ssid = (config) ["ssid"].as<String>();
    String pwd = (config) ["pwd"].as<String>();
    Serial.printf("正在连接到%s\n" , ssid);
    //连接之前扫描一遍附近的所有WiFi，当WiFi存在时才进行连接操作，否则直接跳过连接
    vTaskDelay(4000 / portTICK_PERIOD_MS);
    int i = WiFi.scanComplete();
    for (int j = 0; j < i; j++) {
        Serial.println(WiFi.SSID(j) + ":" + WiFi.RSSI(j));
        vTaskDelay(4000 / portTICK_PERIOD_MS);
    }
reload:
    {
        ssid = (config) ["ssid"].as<String>();
        int i = WiFi.scanComplete();
        for (int j = 0; j < i; j++) {
            Serial.println(WiFi.SSID(j) + ":" + WiFi.RSSI(j));

            if (WiFi.SSID(j) != ssid) {
                continue;
            } else {
                Serial.printf("找到了%s 开始连接\n" , ssid);

                goto hasSSID;
                return;
            }
        }
    }
    Serial.printf("附近没有找到%s,重新开始扫描\n" , ssid);
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    goto reload;
hasSSID:
    {
        WiFi.begin(ssid , pwd);
        while (WiFi.status() != WL_CONNECTED) {

            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }

        Serial.println(WiFi.RSSI());

        String s = WiFi.localIP().toString();
        Serial.printf("已连接到%s：IP:%s\n" , ssid , WiFi.localIP().toString().c_str());
        Serial.flush();
        createWiFiStateTask();
        createTimeUpdateTask();
        // createGetWeatherTask();
        createWsClientTask();
        vTaskDelete(NULL);
        return;
    }
    WiFi.scanNetworks(true);
}

//Ws事件处理
void onEvent(WStype_t type , uint8_t* payload , size_t length) {
    String s = (char*) payload;
    s.trim();
    switch (type) {
    case WStype_TEXT:
    Serial.printf("收到消息:%s\n" , payload);
    commandHandler(s);
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
    WiFi.softAP("wusui_Ya" , "qinsansui233" , 12 , 0);
    vTaskDelete(NULL);
}


void createGetWeatherTask() {
    if (xTaskCreate(
        vTaskCreateWeatherInfo ,
        "getWeather" ,
        20480 ,
        NULL ,
        1 ,
        &initWeatherHandler
    ) != pdPASS) {
        Serial.println("获取天气信息失败");
        vTaskDelete(initWeatherHandler);
    }

}
void vTaskCreateWeatherInfo(void* p) {
    String* info = getWeatherConfig();
    if (info == NULL) {
        Serial.println("天气配置信息为空");

        vTaskDelete(NULL);
        return;
    }
    Serial.println(*info);
    DynamicJsonDocument jsonObj(200);
    deserializeJson(jsonObj , *info);
    delete info;
    String key = jsonObj ["key"].as<String>();
    String location = jsonObj ["location"].as<String>();
    while (1) {
        initWeather(key , location);
        //每30分钟更新一次
        vTaskDelay(1000 * 60 * 30 / portTICK_PERIOD_MS);
    }

    vTaskDelete(NULL);
}

void initWeather(String key , String location) {
    Serial.printf("initWeather查看实际的堆栈使用量%d\n" , uxTaskGetStackHighWaterMark(initWeatherHandler));

    Serial.println("开始请求天气\n" + key + location);
    WiFiClientSecure* secure = new WiFiClientSecure();
    secure->setInsecure();
    HTTPClient* https = new HTTPClient();
    https->setUserAgent(
        "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, "
        "like Gecko) Chrome/109.0.0.0 Safari/537.36");
    https->addHeader("Accept" , "application/json");
    https->addHeader("Connection" , "close");
    https->addHeader("Accept-Encoding" , "none");
    String url =
        "https://devapi.qweather.com/v7/weather/now?gzip=n&key=" + key +
        "&location=" + location;
    Serial.println("url:" + url);
    if (https->begin(*secure , url)) {
        int code = https->GET();
        Serial.println("GET");

        if (code == HTTP_CODE_OK) {
            auto stream = https->getStreamPtr();
            size_t size = https->getSize();
            uint8_t inbuff [size];
            stream->readBytes(inbuff , size);
            uint8_t* outbuf = new uint8_t();
            uint32_t out_size = 0;
            ArduinoUZlib::decompress(
                inbuff ,
                size ,
                outbuf ,
                out_size);
            String* payload = new String((char*) outbuf);
            DynamicJsonDocument* json = new DynamicJsonDocument(payload->length() * 2);
            deserializeJson(*json , *payload);
            JsonObject obj = (*json).as<JsonObject>();

            wa.code = obj ["code"].as<String>();
            wa.tmp = obj ["now"]["temp"].as<String>();
            wa.humidity = obj ["now"]["humidity"].as<String>();
            wa.text = obj ["now"]["text"].as<String>();
            wa.windDir = obj ["now"]["windDir"].as<String>();
            wa.windScale = obj ["now"]["windScale"].as<String>();
            Serial.println("获取到天气:" + *payload);
            delete json;
            delete outbuf;
            delete payload;
        }
    }

    https->end();
    secure->stop();
    delete secure;
    delete https;
}
//WiFi配置接口
//POST ssid,pwd
void webConfigureWiFi(AsyncWebServerRequest* request , JsonVariant& json) {
    DynamicJsonDocument jsonObj = json.as<JsonObject>();
    String content = jsonObj.as<String>();
    DynamicJsonDocument* respJson = new DynamicJsonDocument(100);
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
    delete respJson;
    delete respStr;
}
void configureWeather(AsyncWebServerRequest* request , JsonVariant& json) {
    DynamicJsonDocument jsonObj = json.as<JsonObject>();
    DynamicJsonDocument* respJson = new DynamicJsonDocument(100);
    String key = jsonObj ["key"].as<String>();
    String location = jsonObj ["location"].as<String>();
    String content = jsonObj.as<String>();
    if (key.isEmpty() || location.isEmpty()) {
        (*respJson) ["code"] = -1;
        (*respJson) ["msg"] = "配置信息不完整";
    } else {
        (*respJson) ["code"] = 0;
        (*respJson) ["msg"] = "写入配置成功";
        saveWeatherConfig(content);
        Serial.println("读取到的天气配置" + content);
        createGetWeatherTask();
    }
    String* s = new String();
    deserializeJson(*respJson , *s);
    request->send(200 , "application/json" , *s);
    delete s;
    delete respJson;

}
//获取WiFi配置
String getWifiConfig() {
    File f = SPIFFS.open(wifiPath , "r");
    if (!f) {
        Serial.println("获取WiFi配置失败，将启动热点并开启服务器进行配网操作");
        f.close();
        return "";
    }
    String content = f.readString();
    f.close();
    if (content.isEmpty()) {
        Serial.println("WiFi配置为空");
        return "";
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
String* getWeatherConfig() {
    File f = SPIFFS.open(weatherConfigPath , "r");
    if (!f) {
        Serial.println("获取天气配置失败");
        f.close();
        return NULL;
    }
    String* content = new String();
    *content = f.readString();
    f.close();
    if (content->isEmpty()) {
        Serial.println("天气配置为空");
        return NULL;
    }
    return content;

}
//保存天气接口配置
void saveWeatherConfig(String content) {
    File f = SPIFFS.open(weatherConfigPath , "w+" , true);
    if (!f) {
        Serial.println("不能创建文件");
        return;
    }
    Serial.println("写入天气信息:" + content);
    f.print(content);
    f.flush();
    f.close();
}
void vApplicationStackOverflowHook(TaskHandle_t xTask ,
    signed char* pcTaskName) {
    Serial.printf("任务%s出错！\n" , pcTaskName);
}

void loop() {
    vTaskDelete(NULL);
}