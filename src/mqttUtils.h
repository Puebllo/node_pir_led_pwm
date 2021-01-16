#ifndef MQTT_UTILS_H
#define MQTT_UTILS_H

#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include "variables.h"
#include <ArduinoJson.h>
#include "nodeActions.h"

extern String motionConfigTopic;
extern String lightConfigTopic;

extern String motionStateTopic;
extern String lightStateTopic;
extern String lightCommandTopic;

boolean mqttConnectToBroker();
void publishMessage(const char *topic, String message);
void generateTopics();
void registerDeviceInHomeAssistant();
void removeDeviceFromHomeAssistant();
String generateFullDeviceName(String nodeNameBase);
void mqttUtilsInit(String nodeMac);
void handleClientLoop();
boolean setupWifi();


#endif