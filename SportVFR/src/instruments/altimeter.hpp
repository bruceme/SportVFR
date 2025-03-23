#include <Adafruit_BMP085.h>
Adafruit_BMP085 altimeter;

SimpleKalmanFilter altitudeFilter(2, 2, 0.01);

#define ChCustomerChar0 0

int GetAltitude()
{
  altitudeLast = altitude;
  altitude = (int)(altitudeFilter.updateEstimate(altimeter.readAltitude(((long)(barrow-barrowOffset) * MPaPerBarrow_256) / 256L) * FeetPerMeter));

  // -- vsi indicator --
  int deltaAlt = altitudeLast - altitude;

  auto deltaVsiT = deltaAlt / deltaAltVSI;
  // deltaVsiT = (deltaVsiT > 2) ? 2: ((deltaVsiT <-2) ? -2 : deltaVsiT);
  // vsi = ChCustomerChar0 + 3 + deltaVsiT;
  vsi = (deltaVsiT > 0) ? '^' : ((deltaVsiT < 0) ? 'v' : ' ');

  //vsi = '2' + deltaVsiT; 
  return altitude;
}


byte Level[8] {
  B00000,
  B00000,
  B00000,
  B11111,
  B11111,
  B00000,
  B00000,
  B00000
};

byte UpArrow1[8]
{
  B00000,
  B00000,
  B00100,
  B01110,
  B11011,
  B10001,
  B00000,
  B00000
};

byte UpArrow2[8]
{
  0b00100,
  0b01110,
  0b11011,
  0b10001,
  0b00100,
  0b01110,
  0b11011,
  0b10001
};

const byte DownArrow1[8]
{
  0b00000,
  0b00000,
  0b10001,
  0b11011,
  0b01110,
  0b00100,
  0b00000,
  0b00000
};

const byte DownArrow2[8]
{
  0b10001,
  0b11011,
  0b01110,
  0b00100,
  0b10001,
  0b11011,
  0b01110,
  0b00100
};

#define ChDownArrow1 ChCustomerChar0
#define ChDownArrow2 ChCustomerChar0 + 1
#define ChLevel ChCustomerChar0 + 2
#define ChUpArrow2 ChCustomerChar0 + 3
#define ChUpArrow1 ChCustomerChar0 + 4

void SetupLCDCustomCharacters()
{
  lcd.createChar(ChLevel, (char*)Level); 
  lcd.createChar(ChUpArrow1, (char*)UpArrow1); 
  lcd.createChar(ChUpArrow2, (char*)UpArrow2); 
  lcd.createChar(ChDownArrow1, (char*)DownArrow1); 
  lcd.createChar(ChDownArrow2, (char*)DownArrow2); 

}