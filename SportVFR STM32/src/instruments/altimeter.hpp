#pragma once

#include <SimpleKalmanFilter.h>

SimpleKalmanFilter altitudeFilter(2, 2, 0.01);

#define HX701B_Altimeter

#ifdef HX701B_Altimeter
#include "HX710AB.h"

// HX710AB is __NOT__ I2C - it is a custom 2-wire interface ADC for load cells and pressure sensors

//  adjust pins to your setup
uint8_t dataPin = PB12;
uint8_t clockPin = PB13;

HX710B HX(dataPin, clockPin);

void InitAltimeter()
{
  HX.begin();
  //  adjust to your raw measurements.
  HX.calibrate(50, 0, 100, 80);
}

const float VCC = 3.3; // Supply voltage to sensor
const float SENSOR_SCALE = 0.00125; // Example: 1.25 mV/kPa (adjust per CPG040S datasheet)
const float SEA_LEVEL_PRESSURE = 101325.0; // Pa
const float ADC_SCALE = VCC / 8388607.0; // Convert raw to volts
const float PRESSURE_SCALE = 1000.0 / SENSOR_SCALE; // Convert volts to Pascals directly

int ReadAltitude()
{
  
  long raw = HX.read(false); // Read average ADC value
  float pressure_Pa = raw * ADC_SCALE * PRESSURE_SCALE;
  float altitude = 8434.5f * logf(SEA_LEVEL_PRESSURE / pressure_Pa) * FeetPerMeter;

  return altitude;
}
#endif

#ifdef USE_BMP
#include <Adafruit_BMP085.h>
Adafruit_BMP085 pressureSensor;
void InitAltimeter()
{
  pressureSensor.begin();
}

int ReadAltitude()
{
  return pressureSensor.readAltitude(((long)(barrow - barrowOffset) * MPaPerBarrow_256) / 256L) * FeetPerMeter);
}

#endif


#ifdef USE_MS5611
#include "MS5611.h"
MS5611 pressureSensor(0x77);

void InitAltimeter()
{
  Wire.begin();
  if(!pressureSensor.begin())
    Error = "FI: Altimeter";
}

#define AltPressMax 1088660
#define AltPressMin5k 571820
#define AltPressMin 465633

// standard pressure in pascals at 1000' intervals starting at -2000, final last two are 10000 and 15000 (5k intervals) 
int32_t altConversionTable[] =
{
  1088660,
  1050410,
  1013250,
  977166,
  942129,
  908117,
  875105,
  843073,
  811996,
  781854,
  752624,
  724285,
  696817,
  571820,
  465633
};
const int altConversionTableLen = sizeof(altConversionTable)/sizeof(int32_t);

const int INHG_TO_PA = 3386;   // Conversion factor: inches of mercury to Pascals

int altIndexLast = -1;
int32_t altConvertLast0 = -MAX_INT;
int32_t altConvertLast1 = -MAX_INT;
int32_t altDenTLast;

#define Inspect(v) Serial.print(#v " "); Serial.println(v); 

int calculateAltitude(int32_t pressurePa, int offset_inHg)
{
  // Step 1: Convert offset from inches of mercury to Pascals
  int offsetPa = ((long)offset_inHg * INHG_TO_PA)/10;  

  // Step 2: Adjust pressure with offset
  int32_t adjustedPressure100 = pressurePa*10 + offsetPa;

  byte i;
  // cache the last index, so we can avoid constant LUT scans
  if (adjustedPressure100 > altConvertLast0 || adjustedPressure100 < altConvertLast1)
  {
    i = 0;
    while (adjustedPressure100 < altConversionTable[i])
      i++;
    altIndexLast = i;
    altConvertLast0 = altConversionTable[i-1];
    altConvertLast1 = altConversionTable[i];
    altDenTLast = altConvertLast0 - altConvertLast1;
  }

  int32_t numeratorT = altConvertLast0 - adjustedPressure100;

  int altK = 1000 * (altIndexLast-3);
  int alt = altK + (1000L * numeratorT)/altDenTLast;

  // Inspect(pressurePa);
  // Inspect(offset_inHg);
  // Inspect(offsetPa);
  // Inspect(adjustedPressure100);
  // Inspect(altIndexLast);
  // Inspect(altConvertLast0);
  // Inspect(altConvertLast1);
  // Inspect(altK);
  // Inspect(alt);

  return alt;
}

int ReadAltitude()
{
  pressureSensor.read(); 
  int32_t press = (int32_t)pressureSensor.getPressurePascal();
  return calculateAltitude(press, barrowOffset - barrow);
}
#endif

#ifdef USE_GZP6816D
#include "../GZP6816D.hpp"

int ReadAltitude()
{
  Get_GZP6816D_Data();
  return 0;
}
#endif



int GetAltitude()
{
  altitudeLast = altitude;
  altitude =  (int)altitudeFilter.updateEstimate(ReadAltitude());
  // -- vsi indicator --
  int deltaAlt = altitudeLast - altitude;

  auto deltaVsiT = deltaAlt / deltaAltVSI;
  // deltaVsiT = (deltaVsiT > 2) ? 2: ((deltaVsiT <-2) ? -2 : deltaVsiT);
  // vsi = ChCustomerChar0 + 3 + deltaVsiT;
  vsi = (deltaVsiT > 0) ? '^' : ((deltaVsiT < 0) ? 'v' : ' ');

  // vsi = '2' + deltaVsiT;
  return altitude;
}





