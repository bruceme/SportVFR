#include <SimpleKalmanFilter.h>

SimpleKalmanFilter altitudeFilter(2, 2, 0.01);

#define ChCustomerChar0 0

#define USE_MS5611

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

byte Level[8]{
    B00000,
    B00000,
    B00000,
    B11111,
    B11111,
    B00000,
    B00000,
    B00000};

byte UpArrow1[8]{
    B00000,
    B00000,
    B00100,
    B01110,
    B11011,
    B10001,
    B00000,
    B00000};

byte UpArrow2[8]{
    0b00100,
    0b01110,
    0b11011,
    0b10001,
    0b00100,
    0b01110,
    0b11011,
    0b10001};

const byte DownArrow1[8]{
    0b00000,
    0b00000,
    0b10001,
    0b11011,
    0b01110,
    0b00100,
    0b00000,
    0b00000};

const byte DownArrow2[8]{
    0b10001,
    0b11011,
    0b01110,
    0b00100,
    0b10001,
    0b11011,
    0b01110,
    0b00100};

    const byte Knots[8]{
      0b10010,
      0b10100,
      0b11000,
      0b10100,
      0b10010,
      0b00111,
      0b00010,
      0b00010};
    
    const byte GPH[8]{
      0b01100,
      0b10000,
      0b10100,
      0b01100,
      0b00000,
      0b00101,
      0b00111,
      0b00101};

#define ChDownArrow1 ChCustomerChar0
#define ChDownArrow2 ChCustomerChar0 + 1
#define ChLevel ChCustomerChar0 + 2
#define ChUpArrow2 ChCustomerChar0 + 3
#define ChUpArrow1 ChCustomerChar0 + 4

void SetupLCDCustomCharacters()
{
  lcd.createChar(ChLevel, (char *)Level);
  lcd.createChar(ChUpArrow1, (char *)UpArrow1);
  lcd.createChar(ChUpArrow2, (char *)UpArrow2);
  lcd.createChar(ChDownArrow1, (char *)DownArrow1);
  lcd.createChar(ChDownArrow2, (char *)DownArrow2);
}