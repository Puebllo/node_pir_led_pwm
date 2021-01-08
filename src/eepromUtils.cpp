#include "eepromUtils.h"

void saveToEEPROM() {
    StaticJsonDocument<EEPROM_SIZE> data;

    data["1"] = defBrightnessPercent;
    data["2"] = lightTimeSeconds;
    data["3"] = fadeInMs;
    data["4"] = fadeOutMs;
    data["5"] = ap_ssid;
    data["6"] = ap_password;
    data["7"] = nodeNameBase;
    data["8"] = mqtt_server;
    data["9"] = mqtt_port;
    data["0"] = mqtt_user;
    data["A"] = mqtt_pwd;
    data["B"] = 0;  // Factory reset; 0 - false, 1 - true
    char dataAr[EEPROM_SIZE];
    serializeJson(data, Serial);
    delay(1000);
    serializeJson(data, dataAr);

    for (int i = 0; i < EEPROM_SIZE; i++) {
        EEPROM.write(0x0F + i, dataAr[i]);
    }

    boolean ok = EEPROM.commit();
    Serial.println((ok) ? "Commit OK" : "Commit failed");
}

void loadDataFromEEPROM() {
    String json;
    for (int i = 0; i < EEPROM_SIZE; i++) {
        json = json + char(EEPROM.read(0x0F + i));
    }
    Serial.println(json);

    StaticJsonDocument<EEPROM_SIZE> root;

    DeserializationError error = deserializeJson(root, json);

    if (!error) {
        Serial.println("loading data to variables");

        int factoryResetInt = root["B"];
        if (factoryResetInt == 1) {
            factoryReset = true;
            Serial.println("Factory reset requested !");
        } else if (factoryResetInt == 0) {
            factoryReset = false;
        }

        defBrightnessPercent = root["1"];
        lightTimeSeconds = root["2"];
        fadeInMs = root["3"];
        fadeOutMs = root["4"];

        ap_ssid = root["5"].as<String>();
        ap_password = root["6"].as<String>();

        if (!factoryReset) {
            nodeNameBase = root["7"].as<String>();
        }

        mqtt_port = root["9"];

        mqtt_server = root["8"].as<String>();
        mqtt_user = root["0"].as<String>();
        mqtt_pwd = root["A"].as<String>();

  /*       delay(1000);

        Serial.print("defBright");
        Serial.println(defBrightnessPercent);
        Serial.print("lightTime");
        Serial.println(lightTimeSeconds);
        Serial.print("fadeInTime");
        Serial.println(fadeInMs);
        Serial.print("fadeOutTime");
        Serial.println(fadeOutMs);

        Serial.print("ap_ssid");
        Serial.println(ap_ssid);
        Serial.print("ap_password");
        Serial.println(ap_password);

        Serial.print("nodeName");
        Serial.println(nodeNameBase);
        Serial.print("mqttServer");
        Serial.println(mqtt_server);
        Serial.print("mqttPort");
        Serial.println(mqtt_port);
        Serial.print("mqttUser");
        Serial.println(mqtt_user);
        Serial.print("mqttPwd");
        Serial.println(mqtt_pwd); */
    }
}

void clearEEPROM() {
    StaticJsonDocument<EEPROM_SIZE> data;

    data["1"] = 80;
    data["2"] = 15;
    data["3"] = 8;
    data["4"] = 12;
    data["5"] = "";
    data["6"] = "";
    data["7"] = "";
    data["8"] = "";
    data["9"] = "";
    data["0"] = "";
    data["A"] = "";
    data["B"] = 1;

    char dataAr[EEPROM_SIZE];

    serializeJson(data, dataAr);

    for (int i = 0; i < EEPROM_SIZE; i++) {
        EEPROM.write(0x0F + i, dataAr[i]);
    }

    boolean ok = EEPROM.commit();
    Serial.println((ok) ? "Commit OK" : "Commit failed");
}