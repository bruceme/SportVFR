#pragma once

#include <Arduino.h>
#include <SimpleKalmanFilter.h>
#include <EEPROM.h>
#include "header.hpp"

class FuelMap
{
public:
    bool Disabled = false;
    int FlashIndex;
    int ReadPin;

    int lowFuelAlarm;
    int lowFuelWarning;

    int Range[8];
    int Domain[8];

    FuelMap(int readPin, int flashIndex)
    {
        ReadPin = readPin;
        FlashIndex = flashIndex;
//        ReadEEProm();   
    }

    void SetRangeDomain(int range[8], int domain[8], int lowWarning, int lowAlarm)
    {
        memcpy(Range, range, sizeof(int) * 8);
        memcpy(Domain, domain, sizeof(int) * 8);
        lowFuelWarning = lowWarning;
        lowFuelAlarm = lowAlarm;
        WriteEEProm();
    }
    
    void WriteEEProm()
    {
        // EEPROM.put(FlashIndex, Range);
        // EEPROM.put(FlashIndex + sizeof(Range), Domain);
    }

    void ReadEEProm()
    {
        // EEPROM.get(FlashIndex, Range);
        // EEPROM.get(FlashIndex + sizeof(Range), Domain);
    }

    AlarmState Read()
    {
        int inT = RawRead();

        // find the range
        int i;
        for (i = 0; i < 7; i++)
        {
            if (inT < Range[i + 1])
                break;
        }

        // linear interpolate
        int outT = Domain[i];
        if (i < 7)
        {
            outT += (int)((long)(Domain[i + 1] - Domain[i]) * (long)(inT - Range[i]) / (long)(Range[i + 1] - Range[i]));
        }

        if (outT <= lowFuelAlarm)
            return LOW_ALARM;
        if (outT <= lowFuelWarning)
            return LOW_WARNING;
        else
            return NONE;
    }

    int RawRead()
    {
        int inT = analogRead(ReadPin);
        return (inT < ADC_MAX) ? inT : ADC_MAX;
    }

    void CalibratePoint(int pointIndex, int inValue, int outValue)
    {
        if (pointIndex < 0 || pointIndex >= 8)
            return;

        Range[pointIndex] = inValue;
        Domain[pointIndex] = outValue;

        WriteEEProm();
    }
};