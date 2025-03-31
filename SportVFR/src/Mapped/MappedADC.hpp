#include "MappedBase.hpp"

class MappedADC: public MappedBase
{
public:
  byte ReadPin;

  MappedADC(byte readPin, byte flashIndex) : MappedBase(flashIndex)
  {
    ReadPin = readPin;
  }

  int RawRead(int maxVal) override
  {
    int inT = analogRead(ReadPin);
    return (inT > maxVal) ? maxVal :0;
  }
};