#include "MappedBase.hpp"

#define PARAM_OFFSET 4

MappedBase::MappedBase(byte flashIndex) : filter(SimpleKalmanFilter(1.0f, 1.0f, 0.01))
{
    FlashIndex = flashIndex;
    get();
}

void MappedBase::CalibrateLow(int outValue)
{
    in0 = RawRead();
    out0 = outValue;
    Calibrate(in0, out0, in1, out1);
}

void MappedBase::CalibrateHigh(int outValue, int deltaRead)
{
    in1 = RawRead() + deltaRead;
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

AlarmState MappedBase::Read()
{
    auto inT = RawRead();
    if (inT < _permValues.minIn || inT > _permValues.maxIn)
    {
        return otherAlarmState = BAD_DATA_ALARM;
    }
    else
    {
        otherAlarmState = NONE;
    }

    _lastRead = filter.updateEstimate((int)((long)_permValues.slopeNum * (long)inT / (long)_permValues.slopeDen) + _permValues.yInt);

    return getAlarmState();
}

int MappedBase::QuickRead()
{
    return _lastRead;
}

void MappedBase::get() { EEPROM.get(FlashIndex * sizeof(mappedPerm) + PARAM_OFFSET, _permValues); }
void MappedBase::put() { EEPROM.put(FlashIndex * sizeof(mappedPerm) + PARAM_OFFSET, _permValues); }

void MappedBase::SetAlarmLimits(int alarmHigh, int alarmLow, int warnHigh, int warnLow)
{
    _permValues.alarmHigh = alarmHigh;
    _permValues.alarmLow = alarmLow;
    _permValues.warnHigh = warnHigh;
    _permValues.warnLow = warnLow;
//    put();
}

AlarmState MappedBase::getAlarmState()
{
    if (Disabled)
        return NONE;

    if (_lastRead >= _permValues.alarmHigh)
        return HIGH_ALARM;
    else if (_lastRead <= _permValues.alarmLow)
        return LOW_ALARM;
    else if (_lastRead >= _permValues.warnHigh)
        return HIGH_WARNING;
    else if (_lastRead <= _permValues.warnLow)
        return LOW_WARNING;
    else
        return otherAlarmState;
}   