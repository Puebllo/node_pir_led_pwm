/// In this file you can change behavior of controller

#include "nodeActions.h"

unsigned long lastTrigger = 0;
unsigned long clientCheckIn = 0;

boolean motionDetected = false;
boolean shouldReboot = false;

float mapDouble(double x, double in_min, double in_max, double out_min, double out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void setBrightness(int percent) {
    brightnessPercent = percent;
    brightnessPWM = mapDouble(percent, 0, 100, 0, 1024);
}

void fadeIn(int brightnessPecent) {
    for (int i = 0; i <= brightnessPecent; i++) {
        int step = mapDouble(i, 0, 100, 0, 1024);
        analogWrite(ledPin, step);
        delay(fadeInMs);
    }
}

void fadeOut(int brightnessPecent) {
    for (int i = brightnessPecent; i >= 0; i--) {
        int step = mapDouble(i, 0, 100, 0, 1024);
        analogWrite(ledPin, step);
        delay(fadeOutMs);
    }
}

void publishLightState(String incomingLightState) {
    StaticJsonDocument<64> lightMsg;

    lightMsg["brightness"] = brightnessPercent;

    String outState = "OFF";
    if (motionDetected || incomingLightState.equals("ON")){
        outState = "ON";
    }

    lightMsg["state"] = outState;

    char buffer_light[64];

    serializeJson(lightMsg, buffer_light);
    publishMessage(lightStateTopic.c_str(), buffer_light);
}

void callbackAction(char* topic, String messageStr) {
    if (String(topic) == lightCommandTopic) {
        StaticJsonDocument<64> root;

        DeserializationError error = deserializeJson(root, messageStr);

        if (!error) {
            int brightness = brightnessPercent;
            if (root.containsKey("brightness")) {
                brightness = root["brightness"];
            }
            String outState = root["state"];

            setBrightness(brightness);

            if (outState == "ON") {
                fadeIn(brightnessPercent);
            } else if (outState == "OFF") {
                fadeOut(brightnessPercent);
            }

            publishLightState(outState);
        }
    }
}

void publishMotion(boolean motionDetected) {
    StaticJsonDocument<32> motionMsg;
    String msg = "OFF";

    if (motionDetected) {
        msg = "ON";
    }

    motionMsg["motion"] = msg;

    char bufferMotion[32];

    serializeJson(motionMsg, bufferMotion);

    publishMessage(motionStateTopic.c_str(), bufferMotion);
    publishLightState("");
}

void nodeActionLoop() {
    if (!motionDetected && digitalRead(motionSensor) == HIGH) {
        Serial.println("MOTION DETECTED!!!");
        motionDetected = true;
        lastTrigger = millis();
        publishMotion(motionDetected);
        fadeIn(brightnessPercent);
    }

    // Turn off the LED after the number of seconds defined in the lightTimeSeconds variable
    if (motionDetected && (now - lastTrigger > (lightTimeSeconds * SECOND_DEFINITION))) {
        if (digitalRead(motionSensor) == LOW) {
            Serial.println("Motion stopped...");
            motionDetected = false;
            publishMotion(motionDetected);
            fadeOut(brightnessPercent);
        } else {
            Serial.println("Still motion. Adding 10 more seconds");
            lastTrigger = millis();
        }
    }
}