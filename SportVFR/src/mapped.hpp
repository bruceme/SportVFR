struct _mappedPerm
{
  int slopeDen; // slope denominator
  int slopeNum; // slope numerator
  int yInt;
  int minIn;
  int maxIn;
};

class Mapped
{
private:
  _mappedPerm _permValues;

  byte ReadPin;
  byte FlashIndex;
  bool tooHigh = 0;
  bool tooLow = 0;

  SimpleKalmanFilter filter;

  int in0, in1, out0, out1;

  void get() { EEPROM.get(FlashIndex * sizeof(_mappedPerm) + PARAM_OFFSET, _permValues); }
  void put() { EEPROM.put(FlashIndex * sizeof(_mappedPerm) + PARAM_OFFSET, _permValues); }

public:

  Mapped(byte readPin, byte flashIndex) : filter(SimpleKalmanFilter(2.0f, 2.0f, 0.001))
  {
    ReadPin = readPin;
    FlashIndex = flashIndex;
    get();
  }

  void CalibrateLow(int outValue)
  {
    in0 = analogRead(ReadPin);
    out0 = outValue;
    Calibrate(in0, out0, in1, out1);
  }

  void CalibrateHigh(int outValue, int deltaRead = 0)
  {
    in1 = analogRead(ReadPin) + deltaRead;
    out1 = outValue;
    Calibrate(in0, out0, in1, out1);
  }

  // If you ever have an 8th grader... show them this code, because you sometimes do _RARELY_ use slope-intercept in the real world
  void Calibrate(int in0, int out0, int in1, int out1)
  {
    _permValues.slopeDen = (in1 - in0);
    _permValues.slopeNum = (out1 - out0);
    _permValues.yInt = out0 - (int)((long)_permValues.slopeNum * (long)in0 / (long)_permValues.slopeDen);
    put();
  }

  int Read()
  {
    int inT = analogRead(ReadPin);
    tooLow = inT < _permValues.minIn;
    tooHigh = inT > _permValues.maxIn;
    return filter.updateEstimate((int)((long)_permValues.slopeNum * (long)inT / (long)_permValues.slopeDen) + _permValues.yInt);
  }
};