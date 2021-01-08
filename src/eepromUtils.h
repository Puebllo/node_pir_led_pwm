#ifndef EEPROM_UTILS_H
#define EEPROM_UTILS_H

#include <ArduinoJson.h>
#include <EEPROM.h>
#include "variables.h"

void saveToEEPROM();
void loadDataFromEEPROM();
void clearEEPROM();


#endif