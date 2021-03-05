#ifndef NODE_ACTIONS_H
#define NODE_ACTIONS_H

#include "variables.h"
#include "mqttUtils.h"

extern boolean shouldReboot;

void setBrightness(int percent);
void fadeIn(int brightnessPecent);
void fadeOut(int brightnessPecent);
void callbackAction(String topic, String messageStr);
void publishLightState(String incomingLightState);
void nodeActionLoop();
void publishControllerState();

#endif