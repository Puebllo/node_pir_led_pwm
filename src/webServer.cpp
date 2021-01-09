#include "webServer.h"

AsyncWebServer server(80);


String getParamValue(AsyncWebServerRequest *request, String paramName) {
    if (request->hasParam(paramName, true)) {
        AsyncWebParameter *p = request->getParam(paramName, true);
        Serial.print(paramName);
        Serial.print("= ");
        Serial.println(p->value());
        return p->value();
    }
    return "";
}

void saveDeviceBehaviorDataToEEPROM(AsyncWebServerRequest *request) {
    String value = getParamValue(request, "defBright");
    if (value.length() > 0) {
        defBrightnessPercent = value.toInt();
    }
    value = getParamValue(request, "lightTime");
    if (value.length() > 0) {
        lightTimeSeconds = value.toInt();
    }
    value = getParamValue(request, "fadeInTime");
    if (value.length() > 0) {
        fadeInMs = value.toInt();
    }
    value = getParamValue(request, "fadeOutTime");
    if (value.length() > 0) {
        fadeOutMs = value.toInt();
    }

    saveToEEPROM();

    request->redirect("/");
}

void saveMQTTDataToEEPROM(AsyncWebServerRequest *request) {
    boolean mqttConfChanged = false;
    String value = getParamValue(request, "nodeName");
    if ((value.length() > 0) & (!value.equals(nodeNameBase))) {
        generateTopics();
        nodeNameBase = value.c_str();
        generateFullDeviceName(nodeNameBase);
    }

    value = getParamValue(request, "mqttServer");
    if ((value.length() > 0) & (!value.equals(mqtt_server))) {
        mqtt_server = value.c_str();
        mqttConfChanged = true;
    }
    value = getParamValue(request, "mqttPort");
    if ((value.length() > 0) & (value.toInt() != mqtt_port)) {
        mqtt_port = value.toInt();
        mqttConfChanged = true;
    }
    value = getParamValue(request, "mqttUser");
    if ((value.length() > 0) & (!value.equals(mqtt_user))) {
        mqtt_user = value.c_str();
        mqttConfChanged = true;
    }
    value = getParamValue(request, "mqttPwd");
    if ((value.length() > 0) & (!value.equals(mqtt_pwd))) {
        mqtt_pwd = value.c_str();
        mqttConfChanged = true;
    }

    saveToEEPROM();
    request->redirect("/");

    if (mqttConfChanged) {
        Serial.println("MQTT config changed. reboot !");
        delay(10);
        ESP.restart();
    }
}

void saveDeviceAPDataToEEPROM(AsyncWebServerRequest *request) {
    String value = getParamValue(request, "apSSID");
    if (value.length() > 0) {
        ap_ssid = value;
    }

    value = getParamValue(request, "apPwd");
    if (value.length() > 0) {
        ap_password = value;
    }

    saveToEEPROM();

    request->redirect("/");
}

void doFactoryReset() {
    clearEEPROM();

    delay(500);
    ESP.restart();
}

String processor(const String &var) {
    if (var == "MAC") {
        return WiFi.macAddress();
    }
    if (var == "IP") {
        return WiFi.localIP().toString();
    }
    if (var == "DEF_BRT") {
        return String(defBrightnessPercent);
    }
    if (var == "LIGHT_TIME") {
        return String(lightTimeSeconds);
    }
    if (var == "FADE_IN") {
        return String(fadeInMs);
    }
    if (var == "FADE_OUT") {
        return String(fadeOutMs);
    }
    if (var == "NODE_NAME") {
        return nodeNameBase;
    }
    if (var == "MQTT_SERVER") {
        return mqtt_server;
    }
    if (var == "MQTT_PORT") {
        return String(mqtt_port);
    }
    if (var == "MQTT_USER") {
        return mqtt_user;
    }
    if (var == "AP_SSID") {
        return ap_ssid;
    }

    return String();
}
void otaUpdate() {
    server.on(
        "/update", HTTP_POST, [](AsyncWebServerRequest *request) {
            shouldReboot = !Update.hasError();
            AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", shouldReboot ? "OK. Will reboot now" : "FAIL");
            response->addHeader("Connection", "close");
            request->send(response); },
        [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
            if (!index) {
                Serial.printf("Update Start: %s\n", filename.c_str());
                Update.runAsync(true);
                if (!Update.begin((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000)) {
                    Update.printError(Serial);
                }
            }
            if (!Update.hasError()) {
                if (Update.write(data, len) != len) {
                    Update.printError(Serial);
                }
            }
            if (final) {
                if (Update.end(true)) {
                    Serial.printf("Update Success: %uB\n", index + len);
                } else {
                    Update.printError(Serial);
                }
            }
        });
}

void startServer() {
    Serial.println("Starting web server at port 80");

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", index_html, processor);
        request->send(response);
    });

    server.on("/devBehConfig", HTTP_POST, [](AsyncWebServerRequest *request) {
        saveDeviceBehaviorDataToEEPROM(request);
    });

    server.on("/mqttConfig", HTTP_POST, [](AsyncWebServerRequest *request) {
        saveMQTTDataToEEPROM(request);
    });

    server.on("/devConfig", HTTP_POST, [](AsyncWebServerRequest *request) {
        saveDeviceAPDataToEEPROM(request);
    });

    server.on("/factoryReset", HTTP_POST, [](AsyncWebServerRequest *request) {
        doFactoryReset();
        request->redirect("/");
    });

    server.on("/discoverHA", HTTP_POST, [](AsyncWebServerRequest *request) {
        request->redirect("/");

        registerDeviceInHomeAssistant();
    });
    server.on("/removeHA", HTTP_POST, [](AsyncWebServerRequest *request) {
        removeDeviceFromHomeAssistant();
        request->redirect("/");
    });
    server.on("/reboot", HTTP_POST, [](AsyncWebServerRequest *request) {
        ESP.restart();
        request->redirect("/");
    });
    otaUpdate();
    Serial.println("Registered endpoints");
    server.begin();
    Serial.println("Webserver started");
}