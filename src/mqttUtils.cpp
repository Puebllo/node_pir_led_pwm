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

void handleClientLoop(){
        client.loop();
          if (!client.connected() && lastReconnectAttempt == 0) {
        Serial.println("Disconnected from mqtt broker");
        lastReconnectAttempt = now;
    } 

     if (lastReconnectAttempt != 0 && (now - lastReconnectAttempt >= 30000)) {
        if (mqttConnectToBroker()) {
            lastReconnectAttempt = 0;
        } else {
            lastReconnectAttempt = now;
        }
    } 
}
