#include "variables.h"
#include <ArduinoJson.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <PubSubClient.h>

#include "views.cpp"

#define EEPROM_SIZE 512

const int led = 12;
const int motionSensor = 14;

int brightnessPWM = 0;
int brightnessPercent = 0;

unsigned long now = millis();
unsigned long lastTrigger = 0;
unsigned long lastReconnectAttempt = 0;
unsigned long clientCheckIn = 0;

boolean startTimer = false;

WiFiClient espClient;
PubSubClient client(espClient);

String nodeName = "";
String nodeMacAddress = "";
int qos = 0;

String motionConfigTopic = "";
String lightConfigTopic = "";

String motionStateTopic = "";
String lightStateTopic = "";
String lightCommandTopic = "";

AsyncWebServer server(80);
boolean shouldReboot = false;

boolean mqttConnectToBroker() {
    //   if (WiFi.status() != WL_CONNECTED){
    //       Serial.println("WIFI DISCONNECTED !!!!");
    //       setup_wifi();
    //   }

    Serial.print("Attempting MQTT connection...");
    if (client.connect(nodeName.c_str(), mqtt_user.c_str(), mqtt_pwd.c_str())) {
        Serial.println("connected");
        client.subscribe(lightCommandTopic.c_str());
        Serial.print("subscribing to: ");
        Serial.println(lightCommandTopic.c_str());
        return true;
    } else {
        Serial.print("failed, rc=");
        Serial.print(client.state());
        Serial.println(" try again in 5 seconds");
        return false;
    }
    return false;
}

void publishMessage(const char *topic, String message) {
    if (!client.connected()) {
        mqttConnectToBroker();
    }
    client.publish(topic, message.c_str(), true);
    Serial.print("Publishing to: ");
    Serial.print(topic);
    Serial.print(" message = ");
    Serial.println(message);
}

void saveToEEPROM() {
    StaticJsonDocument<EEPROM_SIZE> data;

    data["1"] = defBrightnessPercent;
    data["2"] = lightTimeSeconds;
    data["3"] = fadeInMs;
    data["4"] = fadeOutMs;
    data["5"] = ap_ssid;
    data["6"] = ap_password;
    data["7"] = nodeNameBase;
    data["8"] = mqtt_server;
    data["9"] = mqtt_port;
    data["0"] = mqtt_user;
    data["A"] = mqtt_pwd;
    data["B"] = 0;  // Factory reset; 0 - false, 1 - true
    char dataAr[EEPROM_SIZE];
    serializeJson(data, Serial);
    delay(1000);
    serializeJson(data, dataAr);

    for (int i = 0; i < EEPROM_SIZE; i++) {
        EEPROM.write(0x0F + i, dataAr[i]);
    }

    boolean ok = EEPROM.commit();
    Serial.println((ok) ? "Commit OK" : "Commit failed");

    /*    delay(1000);
    Serial.print("ap_ssid");
    Serial.println(ap_ssid);
    Serial.print("ap_password");
    Serial.println(ap_password);

    Serial.print("nodeName");
    Serial.println(nodeNameBase);
    Serial.print("mqttServer");
    Serial.println(mqtt_server);
    Serial.print("mqttPort");
    Serial.println(mqtt_port);
    Serial.print("mqttUser");
    Serial.println(mqtt_user);
    Serial.print("mqttPwd");
    Serial.println(mqtt_pwd); */
}

void loadDataFromEEPROM() {
    String json;
    for (int i = 0; i < EEPROM_SIZE; i++) {
        json = json + char(EEPROM.read(0x0F + i));
    }
    Serial.println(json);

    StaticJsonDocument<EEPROM_SIZE> root;

    DeserializationError error = deserializeJson(root, json);

    if (!error) {
        Serial.println("loading data to variables");

        int factoryResetInt = root["B"];
        if (factoryResetInt == 1) {
            factoryReset = true;
            Serial.println("Factory reset requested !");
        } else if (factoryResetInt == 0) {
            factoryReset = false;
        }

        defBrightnessPercent = root["1"];
        lightTimeSeconds = root["2"];
        fadeInMs = root["3"];
        fadeOutMs = root["4"];

        ap_ssid = root["5"].as<String>();
        ap_password = root["6"].as<String>();

        if (!factoryReset) {
            nodeNameBase = root["7"].as<String>();
        }

        mqtt_port = root["9"];

        mqtt_server = root["8"].as<String>();
        mqtt_user = root["0"].as<String>();
        mqtt_pwd = root["A"].as<String>();

        /*         delay(1000);
        Serial.print("ap_ssid");
        Serial.println(ap_ssid);
        Serial.print("ap_password");
        Serial.println(ap_password);

        Serial.print("nodeName");
        Serial.println(nodeNameBase);
        Serial.print("mqttServer");
        Serial.println(mqtt_server);
        Serial.print("mqttPort");
        Serial.println(mqtt_port);
        Serial.print("mqttUser");
        Serial.println(mqtt_user);
        Serial.print("mqttPwd");
        Serial.println(mqtt_pwd); */
    }
}

String getParamValue(AsyncWebServerRequest *request, String paramName) {
    if (request->hasParam(paramName, true)) {
        AsyncWebParameter *p = request->getParam(paramName, true);
        Serial.print(paramName);
        Serial.print("= ");
        Serial.println(p->value());
        return p->value();
    }
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

    /*    Serial.print("defBright");
    Serial.println(defBrightnessPercent);
    Serial.print("lightTime");
    Serial.println(lightTimeSeconds);
    Serial.print("fadeInTime");
    Serial.println(fadeInMs);
    Serial.print("fadeOutTime");
    Serial.println(fadeOutMs); */

    saveToEEPROM();

    request->redirect("/");
}

String generateFullDeviceName() {
    String mac = WiFi.macAddress();
    mac.replace(":", "");
    int len = mac.length();
    Serial.print("Node name: ");
    Serial.println(nodeName);

    return nodeNameBase + "_" + mac.substring(len - 5, len);
}

void generateTopics() {
    motionConfigTopic = "homeassistant/binary_sensor/" + nodeName + "/pir/config";
    lightConfigTopic = "homeassistant/light/" + nodeName + "/config";

    motionStateTopic = "homeassistant/binary_sensor/" + nodeName + "/pir";
    lightStateTopic = "homeassistant/light/" + nodeName;
    lightCommandTopic = "homeassistant/light/" + nodeName + "/set";

    Serial.println("po przekonwertowaniu");
    Serial.println(motionConfigTopic);
    Serial.println(lightConfigTopic);
    Serial.println(motionStateTopic);
    Serial.println(lightStateTopic);
    Serial.println(lightCommandTopic);

    Serial.println("koniec po przekonwertowaniu");
}

void saveMQTTDataToEEPROM(AsyncWebServerRequest *request) {
    boolean mqttConfChanged = false;
    String value = getParamValue(request, "nodeName");
    if ((value.length() > 0) & (!value.equals(nodeNameBase))) {
        generateTopics();
        nodeNameBase = value.c_str();
        nodeName = generateFullDeviceName();
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
    StaticJsonDocument<EEPROM_SIZE> data;

    data["1"] = 80;
    data["2"] = 15;
    data["3"] = 8;
    data["4"] = 12;
    data["5"] = "";
    data["6"] = "";
    data["7"] = "";
    data["8"] = "";
    data["9"] = "";
    data["0"] = "";
    data["A"] = "";
    data["B"] = 1;

    char dataAr[EEPROM_SIZE];

    serializeJson(data, dataAr);

    for (int i = 0; i < EEPROM_SIZE; i++) {
        EEPROM.write(0x0F + i, dataAr[i]);
    }

    boolean ok = EEPROM.commit();
    Serial.println((ok) ? "Commit OK" : "Commit failed");

    delay(500);
    ESP.restart();
}

void removeDeviceFromHomeAssistant() {
    publishMessage(motionConfigTopic.c_str(), "");
    delay(500);
    publishMessage(lightConfigTopic.c_str(), "");
    delay(500);
}

void registerDeviceInHomeAssistant() {
    //register motion
    StaticJsonDocument<512> motionMsg;

    motionMsg["name"] = nodeName;
    motionMsg["device_class"] = "motion";
    motionMsg["state_topic"] = motionStateTopic;
    motionMsg["unique_id"] = nodeName + "-pir";
    motionMsg["qos"] = qos;
    motionMsg["value_template"] = "{{ value_json.motion }}";

    JsonObject data = motionMsg.createNestedObject("device");

    JsonArray identData = data.createNestedArray("identifiers");
    identData.add(nodeMacAddress);

    data["name"] = nodeName;
    data["model"] = "Node-01";
    data["manufacturer"] = "Pueblo";

    char buffer_motion[512];

    serializeJson(motionMsg, buffer_motion);

    Serial.println(motionConfigTopic);
    Serial.println(buffer_motion);

    publishMessage(motionConfigTopic.c_str(), buffer_motion);
    delay(500);

    //register light
    StaticJsonDocument<512> lightMsg;

    lightMsg["name"] = nodeName + "-light";
    lightMsg["platform"] = "mqtt";
    lightMsg["state_topic"] = lightStateTopic;
    lightMsg["command_topic"] = lightCommandTopic;
    lightMsg["unique_id"] = nodeName + "-light";
    lightMsg["brightness"] = true;
    lightMsg["brightness_scale"] = 100;
    lightMsg["color_temp"] = false;
    lightMsg["schema"] = "json";
    lightMsg["qos"] = qos;

    JsonObject data1 = lightMsg.createNestedObject("device");

    JsonArray identData1 = data1.createNestedArray("identifiers");
    identData1.add(nodeMacAddress);

    data1["name"] = nodeName;
    data1["model"] = "Node-01";
    data1["manufacturer"] = "Pueblo";

    char buffer_light[512];

    serializeJson(lightMsg, buffer_light);

    publishMessage(lightConfigTopic.c_str(), buffer_light);
    delay(500);
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

void setup_wifi() {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
    boolean problemWithConnection = false;

    if (ap_ssid.length() > 0 && ap_password.length() > 0) {
        delay(10);
        Serial.println();
        Serial.print("Connecting to ");
        Serial.println(ap_ssid);

        WiFi.setSleepMode(WIFI_NONE_SLEEP);

        WiFi.begin(ap_ssid, ap_password);

        int attempt = 1;
        while (WiFi.status() != WL_CONNECTED) {
            delay(500);
            Serial.print(".");
            if (attempt == 60) {
                problemWithConnection = true;
                break;
            }

            attempt++;
        }

        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("");
            Serial.println("WiFi connected");
            Serial.println("IP address: ");
            Serial.println(WiFi.localIP());
        }
    } else {
        problemWithConnection = true;
    }

    if (problemWithConnection) {
        Serial.println("Setting node in AP mode");
        IPAddress localIp(192, 168, 1, 1);
        IPAddress gateway(192, 168, 1, 1);
        IPAddress subnet(255, 255, 255, 0);

        WiFi.softAP("NODE", "12345678");
        WiFi.softAPConfig(localIp, gateway, subnet);
    }
}

float mapDouble(double x, double in_min, double in_max, double out_min, double out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void setBrightness(int percent) {
    brightnessPercent = percent;
    brightnessPWM = mapDouble(percent, 0, 100, 0, 1024);
}

void fadeIn(int ledPin, int brightnessPecent) {
    for (int i = 0; i <= brightnessPecent; i++) {
        int step = mapDouble(i, 0, 100, 0, 1024);
        analogWrite(ledPin, step);
        delay(fadeInMs);
    }
}

void fadeOut(int ledPin, int brightnessPecent) {
    for (int i = brightnessPecent; i >= 0; i--) {
        int step = mapDouble(i, 0, 100, 0, 1024);
        analogWrite(ledPin, step);
        delay(fadeOutMs);
    }
}

void publishLightState(String incomingLightState) {
    StaticJsonDocument<64> lightMsg;

    lightMsg["brightness"] = brightnessPercent;

    String outState = "OFF";
    if ((digitalRead(motionSensor) == HIGH) || (incomingLightState == "ON")) {
        outState = "ON";
    }

    lightMsg["state"] = outState;

    char buffer_light[64];

    serializeJson(lightMsg, buffer_light);
    publishMessage(lightStateTopic.c_str(), buffer_light);
}

void callback(char *topic, byte *message, unsigned int length) {
    Serial.print("Message arrived on topic: ");
    Serial.print(topic);
    Serial.print(". Message: ");
    String messageTemp;

    for (int i = 0; i < length; i++) {
        messageTemp += (char)message[i];
    }
    Serial.println(messageTemp);

    if (String(topic) == lightCommandTopic) {
        StaticJsonDocument<64> root;

        DeserializationError error = deserializeJson(root, messageTemp);

        if (!error) {
            int brightness = brightnessPercent;
            if (root.containsKey("brightness")) {
                brightness = root["brightness"];
            }
            String outState = root["state"];

            setBrightness(brightness);

            if (outState == "ON") {
                fadeIn(led, brightnessPercent);
            } else if (outState == "OFF") {
                fadeOut(led, brightnessPercent);
            }

            publishLightState(outState);
        }
    }
}

void mqttUtilsInit(WiFiClient espClient, String nodeNamee, String nodeMac) {
    nodeMacAddress = nodeMac;
    nodeName = nodeNamee;

    generateTopics();

    client.setBufferSize(512);
    client.setServer(mqtt_server.c_str(), mqtt_port);
    client.setCallback(callback);
}

void publishMotion(boolean motionDetected) {
    StaticJsonDocument<32> motionMsg;
    String msg = "OFF";

    if (motionDetected) {
        msg = "ON";
    }

    motionMsg["motion"] = msg;

    char bufferMotion[32];

    serializeJson(motionMsg, bufferMotion);

    publishMessage(motionStateTopic.c_str(), bufferMotion);
    publishLightState(msg);
}

void setup() {
    Serial.begin(115200);
    EEPROM.begin(EEPROM_SIZE);
    delay(200);
    loadDataFromEEPROM();

    pinMode(motionSensor, INPUT_PULLDOWN_16);
    pinMode(led, OUTPUT);

    setBrightness(defBrightnessPercent);
    nodeName = generateFullDeviceName();

    setup_wifi();
    mqttUtilsInit(espClient, nodeName, WiFi.macAddress());

    if (WiFi.status() == WL_CONNECTED) {
        mqttConnectToBroker();
    }

    startServer();
}

void loop() {
    // Current time
    long now = millis();
    client.loop();
    if (!client.connected() && lastReconnectAttempt == 0) {
        Serial.println("Disconnected from mqtt broker");
        lastReconnectAttempt = now;
    }

    if (!startTimer && digitalRead(motionSensor) == HIGH) {
        Serial.println("MOTION DETECTED!!!");
        startTimer = true;
        lastTrigger = millis();
        publishMotion(true);
        fadeIn(led, brightnessPercent);
    }

    // Turn off the LED after the number of seconds defined in the lightTimeSeconds variable
    if (startTimer && (now - lastTrigger > (lightTimeSeconds * 1000))) {
        if (digitalRead(motionSensor) == LOW) {
            Serial.println("Motion stopped...");
            startTimer = false;
            publishMotion(false);
            fadeOut(led, brightnessPercent);
        } else {
            Serial.println("Still motion. Adding 10 more seconds");

            lastTrigger = millis();
        }
    }

    if (lastReconnectAttempt != 0 && (now - lastReconnectAttempt >= 30000)) {
        if (mqttConnectToBroker()) {
            lastReconnectAttempt = 0;
        } else {
            lastReconnectAttempt = now;
        }
    }
    if (shouldReboot) {
        delay(300);
        ESP.restart();
    }
}