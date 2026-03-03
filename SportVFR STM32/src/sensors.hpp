#pragma once

#include <Arduino.h>
#include "header.hpp"

AlarmState ReadTachometer();

struct sensor
{
  String name;
  int pin;
  int warnHigh;
  int warnLow;
  int alarmHigh;
  int alarmLow;
  byte displayDigits;
  char pad;
  // int Value() { return lastValue; }
  // int RawValue() { return analogRead(pin); } 
  AlarmState (*readCore)(sensor *thisSensor);
  SimpleKalmanFilter filter = SimpleKalmanFilter(1.0f, 1.0f, 0.00033f);
  int lastValue = 0;
  AlarmState lastAlarm = NONE;
};

struct multiPoint
{
    short Range[8];
    short Domain[8];
};

extern sensor OilPressure;
extern sensor OilTemperature;
extern sensor FuelPressure;
extern sensor FuelQuantity1;
extern sensor FuelQuantity2;
extern sensor FuelFlow;
extern sensor Tachometer; 
extern sensor Altimeter;

extern sensor *Sensors[]; 

void PollAllSensors();

