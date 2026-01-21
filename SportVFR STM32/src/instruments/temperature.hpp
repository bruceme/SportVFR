#pragma once

#include <OneWire.h>
#include <DallasTemperature.h>

#define OAT_PIN PA4   // DS18B20 data line

OneWire oneWire(OAT_PIN);
DallasTemperature oatSensor(&oneWire);

// Call this once in setup()
void setupOAT()
{
    oatSensor.begin();
}

// Call this whenever you want a fresh reading
float readOAT_C()
{
    oatSensor.requestTemperatures();          // trigger conversion
    return oatSensor.getTempCByIndex(0);      // first (or only) sensor
}

