#pragma once

#include <Arduino.h>
#include <SimpleKalmanFilter.h>
#include <EEPROM.h>
#include "alarm.hpp"

#define MAX_INT 32767

struct mappedPerm
{
  int slopeDen; // slope denominator
  int slopeNum; // slope numerator
  int yInt;
  int minIn;
  int maxIn;
  int warnHigh;
  int warnLow;
  int alarmHigh;
  int alarmLow;
};


class MappedBase
{
private:
  mappedPerm _permValues;

  byte ReadPin;
  AlarmState otherAlarmState = NONE; 
  int _lastRead;

  SimpleKalmanFilter filter;

  int in0, in1, out0, out1;

  void get();
  void put();

  byte FlashIndex;
  AlarmState getAlarmState();

public:
  bool Disabled = false;

  MappedBase(byte flashIndex);

  void CalibrateLow(int outValue);
  void CalibrateHigh(int outValue, int deltaRead = 0);
  void Calibrate(int in0, int out0, int in1, int out1);
  AlarmState Read();
  virtual int RawRead() = 0;
  int QuickRead();

  void SetAlarmLimits(int alarmHigh, int alarmLow, int warnHigh = MAX_INT, int warnLow = -MAX_INT);
};