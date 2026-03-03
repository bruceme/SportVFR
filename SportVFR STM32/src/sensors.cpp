#include <Arduino.h>
#include "header.hpp"
#include "sensors.hpp"

AlarmState SensorBoundsCheck(AlarmState alarm, sensor *s)
{
  if (alarm != NONE)
    return alarm;

  if (s->lastValue >= s->alarmHigh)
    return HIGH_ALARM;
  if (s->lastValue >= s->warnHigh)
    return HIGH_WARNING;
  if (s->lastValue <= s->alarmLow)
    return LOW_ALARM;
  if (s->lastValue <= s->warnLow)
    return LOW_WARNING;

  return NONE;
}

// Fuel Quantity is a linear interpolation of the 8 points in the multiPoint struct, 
// which are populated from the fuel map calibration.  This is manual today, but 
// eventually we can add a calibration routine that will populate the multiPoint 
// struct from user input.
short MapADCToOutput(multiPoint *mp, short adcValue)
{
    auto Range = mp->Range;
    auto Domain = mp->Domain;
    auto inT = adcValue;

    // find the range
    byte i;
    for (i = 0; i < 7; i++)
    {
        if (adcValue < Range[i + 1])
            break;
    }

    // linear interpolate
    int outT = (int)Domain[i];
    if (i < 7)
    {
        outT += (int)((long)(Domain[i + 1] - Domain[i]) * (long)(inT - Range[i]) / (long)(Range[i + 1] - Range[i]));
    }
    return (short)outT;
}

sensor OilPressure = {"OP", OP_PIN, MAX_INT, MIN_OilPress + 10, MAX_INT, MIN_OilPress, 2, '0',
/// @Derivation
///  AskAI: The following measured voltages I'd like to map into 12-bit adc values then 
///         curve fit a function that will convert the adc readAnalog value into a pressure. 
///         Output a function in 10ths of PSI using only integer math.
///         (0psi,0.38v),(40,1.19),(50,1.4),(60,1.74),(70,2.02),(80,2.08),(90,2.55)
    [](sensor *thisSensor) {
        long x = (long)analogRead(thisSensor->pin);

        int psi = (int)(((x << 16) / 1840000) - (((x * x)<<7) / 94336000)) - 8;
    
        thisSensor->lastValue = (psi < 15) ? 0 : psi;

         return NONE;
     }};

#include <stdint.h>

// Fixed constants
#define R_FIXED     672        // ohms
#define LN_A_Q15    226000     // ln(17200) * 10000
#define B_Q15       181        // 0.0181 * 10000

// Fast integer log2 approximation
static int32_t fast_log2_q15(uint32_t x)
{
    int n = 31 - __builtin_clz(x);   // integer part
    x <<= (31 - n);                  // normalize to 1.xxx in Q31

    // fractional approximation: y = x/2^31
    // log2(y) ≈ (y - 1) * 1.4427
    int32_t frac = (int32_t)(x >> 16) - 32768;   // Q15
    int32_t frac_scaled = (frac * 47321) >> 15;  // 1.4427 in Q15

    return (n << 15) + frac_scaled;  // Q15 log2(x)
}

// Convert ADC → temperature in °C using integer math
int thermistor_temp_c_int(uint16_t adc)
{
    // Compute resistance in ohms (integer)
    uint32_t r = (uint32_t)R_FIXED * adc / (4095 - adc);

    // ln(R) = log2(R) * ln(2)
    int32_t log2_r_q15 = fast_log2_q15(r);
    int32_t ln_r_q15 = (log2_r_q15 * 22713) >> 15;  // ln(2)=0.693147 in Q15

    // T_F = (ln(A) - ln(R)) / B
    int32_t num = LN_A_Q15 - ln_r_q15;  // still Q15
    int32_t t_f_q15 = (num * 10000) / B_Q15;

    // Convert °F → °C
    int t_f = (int)(t_f_q15 / 10000);
    auto t_c = (t_f - 32) * 5 / 9;

    return t_c;
}

// pin, warnHigh, warnLow, alarmHigh, alarmLow, lastValue, readCore
sensor OilTemperature = {"OT", OT_PIN, MAX_OilTemp - 10, MIN_OilTemp + 10, MAX_OilTemp, MIN_OilTemp, 3, '0',
    [](sensor *thisSensor) {
        thisSensor->lastValue = thermistor_temp_c_int(analogRead(thisSensor->pin));
        return (thisSensor->lastValue < 0) ? DATA_MISSING_ALARM : NONE;       
     }};


sensor FuelPressure = {"FP", FP_PIN, MAX_INT, MIN_FuelPress + 10, MAX_INT, MIN_FuelPress, 3,'0',
    /// @Derivation
    ///  AskAI: The following measured voltages I'd like to map into 12-bit adc values then 
    ///         curve fit a function that will convert the adc readAnalog value into a pressure. 
    ///         Output a function in 10ths of PSI using only integer math.
    ///         (0psi, 0.37v),(15,0.9),(20,1.84),(25,1.92),(30,2.41)
    [](sensor *thisSensor) {
        long x = analogRead(thisSensor->pin);

        // y = -96.26 + 0.2071*x - 0.00002242*x^2
        // 316579 = 2^16 / 0.2071
        // 5709000 = 2^7 / 0.000022420
        float p_tenths = ((x<<16)/316579) - (((x * x)<<7)/5709000) - 96;

        thisSensor->lastValue = (p_tenths < 5) ? 0 : p_tenths;

        return NONE;
     }};


multiPoint fuelQ1mp = {
    .Range = {0, 512, 1024, 1536, 2048, 2560, 3072, 4095},
    .Domain = {0, 21, 43, 64, 86, 107, 129, 150}
};
multiPoint fuelQ2mp = {
    .Range = {0, 512, 1024, 1536, 2048, 2560, 3072, 4095},
    .Domain = {0, 21, 43, 64, 86, 107, 129, 150}
};

sensor FuelQuantity1 = {"<Fuel", FQ1_PIN, MAX_INT, MIN_FuelQuantity+2, MAX_INT, MIN_FuelQuantity,4,'0',
    [](sensor *thisSensor) {
         auto adcValue = analogRead(thisSensor->pin);
         thisSensor->lastValue = MapADCToOutput(&fuelQ1mp, adcValue);
         return NONE;
     }};

sensor FuelQuantity2 = {"Fuel>", FQ2_PIN, MAX_INT, MIN_FuelQuantity+2, MAX_INT, MIN_FuelQuantity,4,'0',
    [](sensor *thisSensor) {
         auto adcValue = analogRead(thisSensor->pin);
         thisSensor->lastValue = MapADCToOutput(&fuelQ2mp, adcValue);
         return NONE;
     }};


uint32_t GetFuelFlowDLPH();

sensor FuelFlow = {"FF", -1, MAX_INT, -MAX_INT, MAX_INT, -MAX_INT, 3,'0',
    [](sensor *thisSensor) {
         thisSensor->lastValue = GetFuelFlowDLPH(); // Replace with actual fuel flow reading
         return NONE;
     }};

 
sensor Tachometer = {"Tach", -1, 2720, -MAX_INT, 2900, -MAX_INT, 4,' ',
    [](sensor *thisSensor) {
         thisSensor->lastValue = ReadTachometer();
         return NONE;
     }};

#include "Adafruit_MCP3421.h"
extern Adafruit_MCP3421 altitudeADC;

sensor Altimeter = {"Alt", -1, MAX_INT, -MAX_INT,MAX_INT,-MAX_INT, 5,' ',
      [](sensor *thisSensor) {
        if (altitudeADC.isReady()) {
                thisSensor->lastValue = altitudeADC.readADC(); // Read ADC value
            }
         return NONE;
     }};

sensor *Sensors[] = {
    &OilPressure,
    &OilTemperature,
    &FuelPressure,
    &FuelQuantity1,
    &FuelQuantity2,
    &FuelFlow,
    &Tachometer,
    &Altimeter
};

String AlarmNames[] = {
  "None",
  "!HIGH!",
  "high",
  "!LOW!",
  "low",
  "BadDat!",
  "BadState!",
  "Missing!"
};


// Don't poll all sensors every loop,
//  Instead spread them out to avoid I2C congestion and get more frequent updates from critical sensors like fuel level and engine temp
#define rollingPollingIntervals 2
byte rollingPollingIndex = 0;

/// @brief Poll all sensors in a phased manner
/// @note This spreads the sensor reads over time to avoid I2C congestion
void PollAllSensors()
{
  for(int i = rollingPollingIndex; 
    i < sizeof(Sensors) / sizeof(Sensors[0]); 
    i += rollingPollingIntervals)
  {
      auto alarm = Sensors[i]->readCore(Sensors[i]);
      Sensors[i]->lastValue = Sensors[i]->filter.updateEstimate(Sensors[i]->lastValue);
      alarm = Sensors[i]->lastAlarm = SensorBoundsCheck(alarm, Sensors[i]);
      
      if (alarm != NONE)
      {
        inError = true;
        Error = Sensors[i]->name + ": " + AlarmNames[alarm];
      }
  }

  rollingPollingIndex = (rollingPollingIndex + 1) % rollingPollingIntervals;
}

