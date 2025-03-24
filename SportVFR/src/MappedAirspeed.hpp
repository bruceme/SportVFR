#include <Arduino.h>
#include <Wire.h>
#include "MappedBase.hpp"

#define ADP_ADDRESS 0x28

class MappedAirspeed : public MappedBase
{
public:

    MappedAirspeed (byte flashOffset) : MappedBase(flashOffset) {}

    uint8_t abpStatus;

    #define ABP_STATUS_NORMAL 0
    #define ABP_STATUS_IN_COMMAND 1
    #define ABP_STATUS_STALE_DATA 2
    #define ABP_STATUS_IN_DIAGNOSTICS 3
        
    int RawRead(int maxVal) override
    {
        Wire.requestFrom(ADP_ADDRESS, (uint8_t)2);
        if (Wire.available())
        {
          uint8_t data_byte_1 = Wire.read();
          uint8_t data_byte_2 = Wire.read();
          abpStatus = data_byte_1 >> 6;
          return (data_byte_1 << 8 | data_byte_2) & 0x3FFF;
        }
        abpStatus = ABP_STATUS_STALE_DATA;
        return 0;
    }
};