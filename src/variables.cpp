#include "variables.h"

int defBrightnessPercent = 80;
int lightTimeSeconds = 15;
int fadeInMs = 8;
int fadeOutMs = 12;
String nodeNameBase = "node_pir";

boolean factoryReset = false;

//DEV
// char* ap_ssid = "HA-IOT";
// char* ap_password = "43!&dgf2sQn@";

// char* mqtt_server = "192.168.1.19";
// uint16_t mqtt_port = 1883;
// char*  mqtt_user = "homeassistant";
// char*  mqtt_pwd = "kieweix6zohna0doh4eigh8joNieVae1to6UuNahfaeNgoosh3ohh9leifai7awe";


String ap_ssid = "";
String ap_password = "";

String mqtt_server = "";
uint16_t mqtt_port;
String  mqtt_user = "";
String  mqtt_pwd = "";

