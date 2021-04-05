#include "eepromUtils.h"
#include "mqttUtils.h"
#include "nodeActions.h"
#include "webServer.h"

void setup() {
    Serial.begin(115200);
    EEPROM.begin(EEPROM_SIZE);
    delay(200);
    loadDataFromEEPROM();


    pinMode(motionSensor, INPUT_PULLDOWN_16);
    pinMode(relayPin, OUTPUT);

    setBrightness(defBrightnessPercent);
    WiFi.disconnect();
    setupWifi();
    mqttUtilsInit(WiFi.macAddress());

    if (WiFi.status() == WL_CONNECTED) {
        if (mqttConnectToBroker()){
            publishLightState("");
            publishControllerInnerState();
        }
    }

    startServer();
}

void loop() {
    nodeActionLoop();
    handleClientLoop();

    if (shouldReboot) {
        delay(300);
        ESP.restart();
    }
}
