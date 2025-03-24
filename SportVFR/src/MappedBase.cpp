#include "MappedBase.hpp"

#define PARAM_OFFSET 4

MappedBase::MappedBase(byte flashIndex) : filter(SimpleKalmanFilter(2.0f, 2.0f, 0.001))
{
    FlashIndex = flashIndex;
    get();
}

void MappedBase::CalibrateLow(int outValue)
{
    in0 = RawRead(MAX_INT);
    out0 = outValue;
    Calibrate(in0, out0, in1, out1);
}

void MappedBase::CalibrateHigh(int outValue, int deltaRead)
{
    in1 = RawRead(MAX_INT) + deltaRead;
    out1 = outValue;
    Calibrate(in0, out0, in1, out1);
}

// If you ever have an 8th grader... show them this code, because you sometimes do _RARELY_ use slope-intercept in the real world
void MappedBase::Calibrate(int in0, int out0, int in1, int out1)
{
    _permValues.slopeDen = (in1 - in0);
    _permValues.slopeNum = (out1 - out0);
    _permValues.yInt = out0 - (int)((long)_permValues.slopeNum * (long)in0 / (long)_permValues.slopeDen);
    put();
}

int MappedBase::Read()
{
    auto inT = RawRead(MAX_INT);
    tooLow = inT < _permValues.minIn;
    tooHigh = inT > _permValues.maxIn;
    return _lastRead = filter.updateEstimate((int)((long)_permValues.slopeNum * (long)inT / (long)_permValues.slopeDen) + _permValues.yInt);
}

int MappedBase::QuickRead()
{
    return _lastRead;
}

void MappedBase::get() { EEPROM.get(FlashIndex * sizeof(_mappedPerm) + PARAM_OFFSET, _permValues); }
void MappedBase::put() { EEPROM.put(FlashIndex * sizeof(_mappedPerm) + PARAM_OFFSET, _permValues); }
