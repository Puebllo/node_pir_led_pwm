#ifndef VARIABLES_H
#define VARIABLES_H

#include <Arduino.h>


// IO
#define EEPROM_SIZE 512
#define SECOND_DEFINITION 1000

//TIMEOUTS
#define WIFI_RECONNECT_TIME_IN_MS 600000
#define MQTT_BROKER_RECONNECT_TIME_IN_MS 300000

extern const int relayPin;
extern const int motionSensor;

extern unsigned long now;
extern unsigned long apModeStartTime;

extern int defBrightnessPercent;
extern int lightTimeSeconds;
extern int fadeInMs;
extern int fadeOutMs;

extern String ap_ssid;
extern String ap_password;

extern String ap_ssid_2;
extern String ap_password_2;

extern String nodeNameBase;
extern String mqtt_server;
extern uint16_t mqtt_port;
extern String  mqtt_user;
extern String  mqtt_pwd;

extern boolean factoryReset;

extern int brightnessPWM;
extern int brightnessPercent;

extern boolean mqttCredsConfigured;

extern String connectedSSID;

#endif