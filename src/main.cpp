#include <main.h>
//变量定义区
TaskHandle_t connectWifiHandler = NULL;
TaskHandle_t createWiFiAPHandler = NULL;
TaskHandle_t createServerHandler = NULL;
TaskHandle_t createWiFiListenerHandler = NULL;
TaskHandle_t createTimeUpdateHandler = NULL;
TaskHandle_t createWiFiScannerHandler = NULL;
TaskHandle_t createIRReceiverHandler = NULL;
TaskHandle_t initScreenHandler = NULL;
TaskHandle_t initWeatherHandler = NULL;
TaskHandle_t readGPSHandler = NULL;
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP , "ntp.ntsc.ac.cn"); // NTP客户端
U8G2_SSD1306_128X64_NONAME_F_HW_I2C  u8g2(U8G2_R0 , /* reset=*/ U8X8_PIN_NONE , 26 , 27);
TinyGPSPlus gps;
void setup() {
    Serial.begin(115200);
    Serial.flush();
    Serial.flush();
    Serial.flush();
    Serial.println("Wellcome");
    if (!SPIFFS.begin()) {
        Serial.println("打开Fs失败");
        Serial.println("打开Fs失败");
        return;
    }
    u8g2.begin();
    u8g2.setFont(u8g2_font_wqy12_t_gb2312);
    u8g2.enableUTF8Print();
    u8g2.begin();
    u8g2.setFont(u8g2_font_wqy12_t_gb2312);
    u8g2.enableUTF8Print();

    WiFi.mode(WIFI_MODE_APSTA);
    WiFi.scanNetworks(true);

    Serial.println("Initialized...");
    createScreenInitialTask();

    Serial.println("Initialized...");
    createScreenInitialTask();
    createConnectWiFiTask();
    createAPTask();
    createServerTask();
    createReadGPSTask();
}
//创建一个屏幕初始化任务
void createScreenInitialTask() {
    if (xTaskCreate(
        vTaskScreenInitial ,
        "initScreen" ,
        2048 ,
        NULL ,
        1 ,
        &initScreenHandler
    ) != pdPASS) {
        Serial.println("初始化屏幕显示失败");
        vTaskDelete(initScreenHandler);
    }
    Serial.printf("createScreenInitialTask查看实际的堆栈使用量%d\n" , uxTaskGetStackHighWaterMark(initScreenHandler));

}
void vTaskReadGPS(void* params) {
    Serial2.begin(9600);

    while (1) {
        if (Serial2.available() > 0) {
            Serial.println("获取地理位置数据");
            if (gps.encode(Serial2.read())) {
                if (gps.location.isValid()) {
                    Serial.print(gps.location.lat() , 6);
                    Serial.print(F(","));
                    Serial.print(gps.location.lng() , 6);
                }


            }
        }
        Serial.println("没有获取到数据");
        wa.locationData = String("没有获取到数据" + timeClient.getFormattedTime()).c_str();
        vTaskDelay(1000 / portTICK_PERIOD_MS);

    }
    vTaskDelete(NULL);

}
void createReadGPSTask() {
    if (xTaskCreate(
        vTaskReadGPS ,
        "readGPS" ,
        2048 ,
        NULL ,
        1 ,
        &readGPSHandler) != pdPASS) {

    }
    Serial.println("创建获取GPS任务");

}
//屏幕初始化
void vTaskScreenInitial(void* params) {

    while (1) {
        draw();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }



    vTaskDelete(NULL);
}
void draw() {
    int width = u8g2.getUTF8Width(timeClient.getFormattedTime().c_str());
    u8g2.clearBuffer();
    //绘制时间
    u8g2.drawUTF8(128 - width , 12 , timeClient.getFormattedTime().c_str());
    //绘制温度
    u8g2.drawUTF8(0 , 12 , String(wa.text + wa.tmp + "°C").c_str());
    u8g2.drawUTF8(0 , 24 , String(wa.windDir + wa.windScale + "级").c_str());
    //绘制地理位置
    u8g2.drawUTF8(0 , 63 , wa.locationData.c_str());
    u8g2.sendBuffer();
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
        vTaskDelete(createServerHandler);
        Serial.println("创建web服务器任务失败");

    }
    Serial.printf("createServerTask查看实际的堆栈使用量%d\n" , uxTaskGetStackHighWaterMark(createServerHandler));

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
        vTaskDelete(connectWifiHandler);
        Serial.println("创建连接WiFi任务失败");
    }
    Serial.printf("createConnectWiFiTask查看实际的堆栈使用量%d\n" , uxTaskGetStackHighWaterMark(connectWifiHandler));
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
        vTaskDelete(createTimeUpdateHandler);
    }
    Serial.printf("createTimeUpdateTask查看实际的堆栈使用量%d\n" , uxTaskGetStackHighWaterMark(createTimeUpdateHandler));
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
    AsyncCallbackJsonWebHandler* lockHandler = new AsyncCallbackJsonWebHandler("/bike/lock" , lockBike);
    AsyncCallbackJsonWebHandler* weatcherHandler = new AsyncCallbackJsonWebHandler("/weather/submit" , configureWeather);
    AsyncCallbackJsonWebHandler* weatcherHandler = new AsyncCallbackJsonWebHandler("/weather/submit" , configureWeather);
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
                (*item) ["ssid"] = ssid;
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
    server.addHandler(lockHandler);
    server.addHandler(wifiConfigureHandler);
    server.addHandler(weatcherHandler);
    server.addHandler(weatcherHandler);
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
    String ssid = (config) ["ssid"].as<String>();
    String pwd = (config) ["pwd"].as<String>();
    Serial.printf("正在连接到%s\n" , ssid);
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
        int i = WiFi.scanComplete();
        for (int j = 0; j < i; j++) {
            Serial.println(WiFi.SSID(j) + ":" + WiFi.RSSI(j));

            if (WiFi.SSID(j) != ssid) {
                continue;
            } else {
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
        //
        if (WiFi.RSSI() > -20) {

        }
        String s = WiFi.localIP().toString();


        Serial.println(WiFi.RSSI());
        //      
        if (WiFi.RSSI() > -20) {

        }
        String s = WiFi.localIP().toString();
        Serial.printf("已连接到%s：IP:%s\n" , ssid , WiFi.localIP().toString().c_str());
        Serial.flush();
        createWiFiStateTask();
        createTimeUpdateTask();
        createGetWeatherTask();
        createGetWeatherTask();
        vTaskDelete(NULL);

    }
    WiFi.scanNetworks(true);
}

//Ws事件处理
void onEvent(AsyncWebSocket* server , AsyncWebSocketClient* client , AwsEventType type , void* arg , uint8_t* data , size_t len) {
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
    WiFi.softAP("wusui_Ya" , "Qinsansui233..." , 12 , 0 , 2);
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

    WiFi.softAP("wusui_Ya" , "Qinsansui233..." , 12 , 0 , 2);
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
    delete respJson;
    delete respStr;
}
void lockBike(AsyncWebServerRequest* request , JsonVariant& json) {
    DynamicJsonDocument jsonObj = json.as<JsonObject>();
    String pwd = jsonObj ["pass"].as<String>();
    if (pwd != "Qinsansui233...") {
        request->send(404);
        return;
    }
    bool needLock = jsonObj ["needLock"].as<bool>();
    Serial.println("needLock" + String(needLock));
    Serial.println("needLock" + String(needLock));
    if (needLock) {
        // digitalWrite(16 , HIGH);
        // digitalWrite(16 , HIGH);
    } else {
        // digitalWrite(16 , LOW);
        // digitalWrite(16 , LOW);

    }
    request->send(200);
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

