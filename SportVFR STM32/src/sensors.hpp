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
  // int Value() { return lastValue; }
  // int RawValue() { return analogRead(pin); } 
  AlarmState (*readCore)(sensor *thisSensor);
  SimpleKalmanFilter filter = SimpleKalmanFilter(1.0f, 1.0f, 0.01);
  int lastValue = 0;
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

extern sensor sensors[]; 

void PollAllSensors();

