#include "variables.h"

//IO
const int ledPin = 12;
const int motionSensor = 14;

unsigned long now = millis();
unsigned long apModeStartTime = 0;

int defBrightnessPercent = 80;
int lightTimeSeconds = 15;
int fadeInMs = 8;
int fadeOutMs = 12;
String nodeNameBase = "node_pir";

boolean factoryReset = false;

String ap_ssid = "";
String ap_password = "";

String ap_ssid_2 = "";
String ap_password_2 = "";

String mqtt_server = "";
uint16_t mqtt_port;
String  mqtt_user = "";
String  mqtt_pwd = "";

int brightnessPWM = 0;
int brightnessPercent = 0;

boolean mqttCredsConfigured = false;

String connectedSSID = "";
