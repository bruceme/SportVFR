
/*-----( Import needed libraries )-----*/
// Get the LCD I2C Library here:
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2); // set the LCD address to 0x27 for a 16 chars and 2 line display

#include <ezButton.h>
ezButton button(6); // 6 on the PCB

String Error = ""; // Error message

#include <EEPROM.h>
#include <SimpleKalmanFilter.h>

#define deltaAltVSI 100

#define BARROW_EEPROP_OFFSET 240

#define MIN_OilTemp 95
#define MAX_OilTemp 245
#define MIN_OilPress 25
#define MAX_OilPress 100
#define MIN_FuelPress 1
#define MAX_FuelPress 8
#define MIN_FuelQuantity 30 // 3.0g
#define ALARM_PIN 8         // On 8 on the PCB

#include "Mapped/MappedADC.hpp"
#include "Mapped/MappedAirspeed.hpp"

MappedAirspeed airspeed(0); // pressure sensor
MappedADC oilPres(A1, 1);
MappedADC oilTemp(A0, 2);
MappedADC fuelPres(A2, 3);
MappedADC fuelQ1(10, 4);
MappedADC fuelQ2(9, 5);
// Mapped auxInput(A3,5); // optional

int heading = 360;
int altitude = 0;
char vsi = '^';
int tachometer = 0;
int lastmillis = millis();
byte revolutions = 0;
int altitudeLast;
bool powerOn = true;
int barrow;
bool up;
bool down;
unsigned long now;
const int sensorCalibration = 3031 - 3034;         // current reference barrometeric pressure - pressure you read when you set your altitude
const int barrowOffset = 2992 + sensorCalibration; // standard + offset for sensor calibration

#define FeetPerMeter 3.28084
#define MPaPerBarrow_256 8669L // barrow is 100 * InHq this 256 bigger than it needs to be (simple math)

#include "configuration.hpp"
#include "encoder.hpp"
#include "utility.hpp"

#include "instruments/altimeter.hpp"
#include "instruments/tach.hpp"
#include "instruments/compass.hpp"

int blinker = 0;

#define EnginePage 0
#define FuelPage 1
#define PageCount 2
int page = EnginePage;

// Use this to display raw sensor output for configuration
// #define SENSOR_CONFIG
int fq1, fq2;
int curDeltaTPhase = 0;

void NormalDisplay()
{
  now = millis();
  auto deltaT = (int)(now - lastmillis);
  auto deltaTPhase = deltaT / 100;
  if (curDeltaTPhase != deltaTPhase)
  {
    curDeltaTPhase = deltaTPhase;
    switch (deltaTPhase)
    {
    case 0:
//      Serial.print('0' + deltaPhase);
      tachometer = ReadTachometer();
      // heading = ReadCompass();
      altitude = GetAltitude();
      break;
    case 1:
//      Serial.print('0' + deltaPhase);
      airspeed.Read();
      oilTemp.Read();
      oilPres.Read();
      fuelPres.Read();
      break;
    case 2:
//      Serial.print('0' + deltaPhase);
      fq1 = fuelQ1.Read();
      fq1 = fq1 < 0 ? 0 : fq1;
      fq2 = fuelQ2.Read();
      fq2 = fq2 < 0 ? 0 : fq2;
      auto fp = fuelPres.QuickRead();
      auto ot = oilTemp.QuickRead();
      auto op = oilPres.QuickRead();

      auto alarmT = (ot > MAX_OilTemp ||
                     ot < MIN_OilTemp ||
                     op > MAX_OilPress ||
                     op < MIN_OilPress ||
                     fp > MAX_FuelPress ||
                     fp < MIN_FuelPress ||
                     fq1 < MIN_FuelQuantity ||
                     fq2 < MIN_FuelQuantity) &&
                    blinker > 1;

      blinker = (blinker + 1) % 4;

      digitalWrite(ALARM_PIN, alarmT ? HIGH : LOW);
      break;
    }
  }
  if (deltaT > 300)
    lastmillis = now;

#ifdef SENSOR_CONFIG
  {
    lcd.setCursor(0, 0);                           // go to the next line
    lcd.print(Format(airspeed.RawRead(), 3, ' ')); // 3
    lcd.print(' ');
    lcd.print(Format(fuelQ1.RawRead(), 3, '0')); // 3
    lcd.print(' ');
    lcd.print(Format(fuelQ2.RawRead(), 3, '0')); // 3
    lcd.print(' ');
    lcd.print(Format(altitude, 4, ' ')); // 15

    lcd.setCursor(0, 1); // go to the next line
    lcd.print("   ");
    lcd.print(Format(oilTemp.RawRead(), 3, '0')); // 8
    lcd.print(' ');
    lcd.print(Format(oilPres.RawRead(), 3, '0')); // 13
    lcd.print(' ');
    lcd.print(Format(fuelPres.RawRead(), 3, '0')); // 16
    return;
  }
#endif

#ifdef SHOW_RAW_COMPASS
  lcd.setCursor(0, 0);
  ShowRawCompass();
  return;
#endif

  lcd.setCursor(0, 0);                                 // go to the next line
  lcd.print(Format(airspeed.QuickRead(), 3, ' '));     // 3
  lcd.print("   ");                                    // 6
  lcd.print(Format(heading - heading % 5, 3, '0'));    // 9
  lcd.print((char)0xdf);                               // 10
  lcd.print(Format(altitude - altitude % 10, 5, ' ')); // 15
  lcd.write(vsi);                                      // 16

  lcd.setCursor(0, 1); // go to the next line
  if (page == EnginePage)
  {
    lcd.print(Format(tachometer, 4, '0'));           // 4
    lcd.print(' ');                                  // 5
    lcd.print(Format(oilTemp.QuickRead(), 3, ' '));  // 8
    lcd.print((char)0xdf);                           // 9 deg
    lcd.print(Format(oilPres.QuickRead(), 4, ' '));  // 13
    lcd.print(Format(fuelPres.QuickRead(), 3, ' ')); // 16
  }
  else if (page == FuelPage)
  {
    lcd.print(Format(fq1 / 10, 2, ' ')); // 2
    lcd.print('.');                      // 3
    lcd.print(Format(fq1 % 10, 1, '0')); // 4
    lcd.print('g');                      // 5
    lcd.print("<fuel>");                 // 11
    lcd.print(Format(fq2 / 10, 2, ' ')); // 13
    lcd.print('.');                      // 14
    lcd.print(Format(fq2 % 10, 1, '0')); // 15
    lcd.print('g');                      // 16

    lcd.print(Format(oilTemp.QuickRead(), 3, ' '));  // 8
    lcd.print((char)0xdf);                           // 9 deg
    lcd.print(Format(oilPres.QuickRead(), 4, ' '));  // 13
    lcd.print(Format(fuelPres.QuickRead(), 3, ' ')); // 16
  }
}
bool inError = false;
void setup()
{
  Serial.begin(9600);

  // Serial is being used for it's interupts :(
  attachTachInt();

  EEPROM.get(BARROW_EEPROP_OFFSET, barrow);
  if (barrow < 2800)
    barrow = 2992;

  // airspeed.Initialize();
//  InitAltimeter();
//  InitCompass();

  lcd.begin(16, 2); // initialize the lcd for 20 chars 4 lines and turn on backlight
  lcd.backlight();
  // SetupLCDCustomCharacters();

  //  encoder.write(barrow*4);

//  pinMode(ALARM_PIN, OUTPUT);

  // InitializeEncoder();

  // CalibrationReset();
  // inError = Error.length() > 0;
  // lcd.clear();
  // lcd.setCursor(6,0);
  // lcd.print("SportVFR");
  // delay(500);
  // Serial.println("Hello");
}

/*--(end setup )---*/

void loop()
{
  button.loop();
  int buttsUp = button.isReleased();

  if (inError)
  {
    lcd.setCursor(0, 0);
    lcd.print(Error.c_str());
    inError = !buttsUp;
    return;
  }

  if (inSetBarrowMode)
  {
    barrow = encoder / 7;

    lcd.setCursor(0, 0); // go to the next line
    lcd.print("Set Pressure Alt");
    lcd.setCursor(0, 1);                     // go to the next line
    lcd.print(Format(barrow / 100, 2, ' ')); // 2
    lcd.print('.');                          // 3
    lcd.print(Format(barrow % 100, 2, '0')); // 5
    lcd.print("      ");
    lcd.print(Format(altitude, 5, ' ')); // 16

    if (buttsUp && inSetBarrowMode)
    {
      inSetBarrowMode = false;
      lcd.clear();
      EEPROM.put(BARROW_EEPROP_OFFSET, barrow);
      encoder = EnginePage;
      return;
    }
    inSetBarrowMode = true;
    altitude = GetAltitude();
  }

  // Detect setup mode select on boot
  if (buttsUp)
  {
    if (powerOn)
      inConfigMode = true;
    else
    {
      inSetBarrowMode = true;
      encoder = barrow * 7;
    }

    powerOn = false;
  }

  if (!inSetBarrowMode)
    NormalDisplay();

  page = (encoder / 4 + PageCount) % PageCount;

  delay(10);
} /* --(end main loop )-- */
