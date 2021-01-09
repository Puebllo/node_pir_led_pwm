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
    pinMode(ledPin, OUTPUT);

    setBrightness(defBrightnessPercent);

    setupWifi();
    mqttUtilsInit(WiFi.macAddress());

    if (WiFi.status() == WL_CONNECTED) {
        mqttConnectToBroker();
    }

    startServer();
}

void loop() {
    nodeActionLoop();
    handleClientLoop();
}
