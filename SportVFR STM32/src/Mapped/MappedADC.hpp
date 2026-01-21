#pragma once

#include <Arduino.h>  // For byte and analogRead
#include "units.hpp"
#include "MappedBase.hpp"

class MappedADC : public MappedBase
{
public:
  byte ReadPin;
  UnitType Units;

  MappedADC(byte readPin, byte flashIndex, UnitType unitType = UNIT_UNITLESS) : 
    MappedBase(flashIndex), ReadPin(readPin), Units(unitType) {}

  int RawRead() override {
    int inT = analogRead(ReadPin);
    return (inT > ADC_MAX) ? ADC_MAX : inT;
  }
};