/// Source code file of mqttUtils
/// This file is responsible for handling low level operations
/// and different border scenarios i.e. reconnecting
/// Most of the the time you don't have to do changes in this file
/// Controller 'heart' is located in nodeActions files

#include "mqttUtils.h"

WiFiClient espClient;
PubSubClient client(espClient);

String motionConfigTopic = "";
String lightConfigTopic = "";

String motionStateTopic = "";
String lightStateTopic = "";
String lightCommandTopic = "";

String nodeName = "";
String nodeMacAddress = "";
int qos = 0;

unsigned long lastReconnectAttempt = 0;

boolean apStarted = false;

boolean setupWifiInner(String ssid, String pwd) {
    boolean toReturn = true;
    if (ssid.length() > 0 && pwd.length() > 0) {
        delay(10);
        Serial.println();
        Serial.print("Connecting to ");
        Serial.print(ssid);
        WiFi.setSleepMode(WIFI_NONE_SLEEP);

        WiFi.begin(ssid, pwd);

        int attempt = 1;
        while (WiFi.status() != WL_CONNECTED) {
            delay(500);
            Serial.print(".");
            if (attempt == 20) {
                toReturn = false;
                break;
            }
            attempt++;
        }
        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("");
            Serial.println("WiFi connected");
            Serial.println("IP address: ");
            Serial.println(WiFi.localIP());
        } else {
            Serial.print("Connection to: ");
            Serial.print(ssid);
            Serial.println(" failed !");
        }
    }
    return toReturn;
}

boolean setupWifi() {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(1000);
    boolean isConnected = false;
    if (ap_ssid.length() > 0 && ap_password.length() > 0) {
        //Try to connect to first AP
        isConnected = setupWifiInner(ap_ssid, ap_password);

        //Try to connect to second AP if configured
        if (!isConnected && ap_ssid_2.length() > 0 && ap_password_2.length() > 0) {
            isConnected = setupWifiInner(ap_ssid_2, ap_password_2);
        }
    }

    //If connection to Wifi failed, then set node in AP mode
    if (!isConnected) {
        now = millis();
        apModeStartTime = now;
        if (!apStarted) {
            WiFi.disconnect();
            delay(1000);

            apStarted = true;

            Serial.println("Setting node in AP mode");
            IPAddress localIp(192, 168, 1, 1);
            IPAddress gateway(192, 168, 1, 1);
            IPAddress subnet(255, 255, 255, 0);

            WiFi.softAP("NODE", "12345678");
            WiFi.softAPConfig(localIp, gateway, subnet);
        }
    } else {
        apModeStartTime = 0;
        apStarted = false;
    }

    return isConnected;
}

boolean mqttConnectToBroker() {
    if (apModeStartTime == 0 && (WiFi.status() != WL_CONNECTED || WiFi.localIP().toString().length() < 6)) {
        setupWifi();
    }
    if (mqttCredsConfigured && WiFi.status() == WL_CONNECTED) {
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
            Serial.print(" try again in ");
            Serial.print(MQTT_BROKER_RECONNECT_TIME_IN_MS);
            Serial.print(" ms");
            return false;
        }
    }

    return false;
}

void publishMessage(const char *topic, String message) {
    client.publish(topic, message.c_str(), true);
    Serial.print("Publishing to: ");
    Serial.print(topic);
    Serial.print(" message = ");
    Serial.println(message);
}

void generateTopics() {
    motionConfigTopic = "homeassistant/binary_sensor/" + nodeName + "/pir/config";
    lightConfigTopic = "homeassistant/light/" + nodeName + "/config";

    motionStateTopic = "homeassistant/binary_sensor/" + nodeName + "/pir";
    lightStateTopic = "homeassistant/light/" + nodeName;
    lightCommandTopic = "homeassistant/light/" + nodeName + "/set";
}

void removeDeviceFromHomeAssistant() {
    publishMessage(motionConfigTopic.c_str(), "");
    delay(1000);
    publishMessage(lightConfigTopic.c_str(), "");
    delay(1000);
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

    publishMessage(motionConfigTopic.c_str(), buffer_motion);

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
}

void callback(char *topic, byte *message, unsigned int length) {
    Serial.print("Message arrived on topic: ");
    Serial.print(topic);
    Serial.print(". Message: ");
    String messageStr;

    for (int i = 0; i < length; i++) {
        messageStr += (char)message[i];
    }
    Serial.println(messageStr);

    callbackAction(topic, messageStr);
}

String generateFullDeviceName(String nodeNameBase) {
    String mac = WiFi.macAddress();
    mac.replace(":", "");
    int len = mac.length();
    Serial.print("Node name: ");
    Serial.println(nodeName);

    return nodeNameBase + "_" + mac.substring(len - 5, len);
}

void mqttUtilsInit(String nodeMac) {
    nodeMacAddress = nodeMac;

    nodeName = generateFullDeviceName(nodeNameBase);

    generateTopics();

    client.setBufferSize(512);
    client.setServer(mqtt_server.c_str(), mqtt_port);
    client.setCallback(callback);
}

void handleClientLoop() {
    now = millis();
    if (client.connected()) {
        client.loop();
    }
    if (mqttCredsConfigured && !client.connected() && lastReconnectAttempt == 0) {
        Serial.println("Disconnected from mqtt broker");
        lastReconnectAttempt = now;
    }
    if (WiFi.localIP().toString().length() < 6 || (apModeStartTime == 0 && WiFi.status() != WL_CONNECTED)) {
        Serial.println("Disconnected from Wi-Fi AP");
        apModeStartTime = now;
    }

    //Try to connect after set time amount
    if (apModeStartTime != 0 && ((now - apModeStartTime) > WIFI_RECONNECT_TIME_IN_MS)) {
        if (setupWifi()) {
            apModeStartTime = 0;
        } else {
            apModeStartTime = now;
        }
    }

    //Try to connect to mqtt broker
    if (lastReconnectAttempt != 0 && ((now - lastReconnectAttempt) > MQTT_BROKER_RECONNECT_TIME_IN_MS)) {
        if (mqttConnectToBroker()) {
            lastReconnectAttempt = 0;
        } else {
            lastReconnectAttempt = now;
        }
    }
}