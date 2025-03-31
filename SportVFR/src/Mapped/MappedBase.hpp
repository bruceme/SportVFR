#pragma once

#include <Arduino.h>
#include <SimpleKalmanFilter.h>
#include <EEPROM.h>

struct _mappedPerm
{
  int slopeDen; // slope denominator
  int slopeNum; // slope numerator
  int yInt;
  int minIn;
  int maxIn;
};


#define MAX_INT 32767

class MappedBase
{
private:
  _mappedPerm _permValues;

  byte ReadPin;
  byte FlashIndex;
  bool tooHigh = 0;
  bool tooLow = 0;
  int _lastRead;

  SimpleKalmanFilter filter;

  int in0, in1, out0, out1;

  void get();
  void put();

public:

  MappedBase(byte flashIndex);

  void CalibrateLow(int outValue);
  void CalibrateHigh(int outValue, int deltaRead = 0);
  // If you ever have an 8th grader... show them this code, because you sometimes do _RARELY_ use slope-intercept in the real world
  void Calibrate(int in0, int out0, int in1, int out1);
  int Read();
  virtual int RawRead(int maxVal) = 0;
  int QuickRead();
};