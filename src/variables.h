#ifndef VARIABLES_H
#define VARIABLES_H

#include <Arduino.h>


// IO
#define EEPROM_SIZE 512

extern const int ledPin;
extern const int motionSensor;

extern unsigned long now;


extern int defBrightnessPercent;
extern int lightTimeSeconds;
extern int fadeInMs;
extern int fadeOutMs;

extern String ap_ssid;
extern String ap_password;

extern String nodeNameBase;
extern String mqtt_server;
extern uint16_t mqtt_port;
extern String  mqtt_user;
extern String  mqtt_pwd;

extern boolean factoryReset;

extern int brightnessPWM;
extern int brightnessPercent;



#endif