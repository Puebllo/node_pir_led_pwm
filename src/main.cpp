#include "eepromUtils.h"
#include "mqttUtils.h"
#include "nodeActions.h"
#include "webServer.h"

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

void setup() {
    Serial.begin(115200);
    EEPROM.begin(EEPROM_SIZE);
    delay(200);
    loadDataFromEEPROM();

    pinMode(motionSensor, INPUT_PULLDOWN_16);
    pinMode(ledPin, OUTPUT);

    setBrightness(defBrightnessPercent);

    setup_wifi();
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
