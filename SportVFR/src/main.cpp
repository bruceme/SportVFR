
/*-----( Import needed libraries )-----*/
// Get the LCD I2C Library here:
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4); // set the LCD address to 0x27 for a 16 chars and 2 line display

#include <ezButton.h>
ezButton button(6); // 6 on the PCB

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
#define ALARM_PIN 9 // On 8 on the PCB

#include "MappedADC.hpp"
#include "MappedAirspeed.hpp"

MappedAirspeed airspeed(0); // pressure sensor
MappedADC oilPres(A1, 1);
MappedADC oilTemp(A0, 2);
MappedADC fuelPres(A2, 3);
MappedADC fuelQ1(10,4);
MappedADC fuelQ2(9,5);
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
const int barrowOffset = 3019 - 2995 - 4;  // current local barrow setting - what SportVFR shows as your altitude

#define FeetPerMeter 3.28084
#define MPaPerBarrow_256 8669L  // barrow is 100 * InHq this 256 bigger than it needs to be (simple math)


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
//#define SENSOR_CONFIG
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
    switch(deltaTPhase)
    {
      case 0:
        tachometer = ReadTachometer();
        heading = ReadCompass();
        altitude = GetAltitude();
        break;
      case 1:
        airspeed.Read();
        oilTemp.Read();
        oilPres.Read();
        fuelPres.Read();
      case 2:
        fq1 = fuelQ1.Read(); fq1 = fq1 < 0 ? 0 : fq1;
        fq2 = fuelQ2.Read(); fq2 = fq2 < 0 ? 0 : fq2;
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
          fq2 < MIN_FuelQuantity 
        ) && blinker > 1;
  
        blinker = (blinker + 1) % 4;
  
        digitalWrite(ALARM_PIN, alarmT ? HIGH : LOW);
      }
  }
  if (deltaT > 300)
    lastmillis = now;

#ifdef  SENSOR_CONFIG
  {
    lcd.setCursor(0, 0);                                 // go to the next line
    lcd.print(Format(airspeed.RawRead(), 3, ' '));       // 3
    lcd.print(' ');
    lcd.print(Format(fuelQ1.RawRead(),3, '0'));      // 3   
    lcd.print(' ');  
    lcd.print(Format(fuelQ2.RawRead(),3, '0'));      // 3 
    lcd.print(' ');  
    lcd.print(Format(altitude, 4, ' ')); // 15

    lcd.setCursor(0, 1);                       // go to the next line
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
  lcd.setCursor(0,0);
  ShowRawCompass();
  return;
#endif

  lcd.setCursor(0, 0);                                 // go to the next line
  lcd.print(Format(airspeed.QuickRead(), 3, ' '));          // 3
  lcd.print("   ");                                    // 6
  lcd.print(Format(heading - heading % 5, 3, '0'));    // 9
  lcd.print((char)0xdf);                               // 10
  lcd.print(Format(altitude - altitude % 10, 5, ' ')); // 15
  lcd.write(vsi);                                      // 16
  
  lcd.setCursor(0, 1);                       // go to the next line
  if (page == EnginePage)
  {
    lcd.print(Format(tachometer, 4, '0'));     // 4
    lcd.print(' ');                            // 5
    lcd.print(Format(oilTemp.QuickRead(), 3, ' ')); // 8
    lcd.print((char)0xdf);                     // 9 deg
    lcd.print(Format(oilPres.QuickRead(), 4, ' ')); // 13
    lcd.print(Format(fuelPres.QuickRead(), 3, ' ')); // 16
  }
  else if (page == FuelPage)
  {
    lcd.print(Format(fq1/10, 2, ' '));     // 2
    lcd.print('.');                           // 3
    lcd.print(Format(fq1%10, 1, '0'));     // 4
    lcd.print('g');                           // 5
    lcd.print("<fuel>");                       // 11
    lcd.print(Format(fq2/10, 2, ' '));     // 13
    lcd.print('.');                           // 14
    lcd.print(Format(fq2%10, 1, '0'));     // 15
    lcd.print('g');                           // 16

    lcd.print(Format(oilTemp.QuickRead(), 3, ' ')); // 8
    lcd.print((char)0xdf);                     // 9 deg
    lcd.print(Format(oilPres.QuickRead(), 4, ' ')); // 13
    lcd.print(Format(fuelPres.QuickRead(), 3, ' ')); // 16
  }
}

void setup()
{
//  Serial.begin(9600);
  attachTachInt();

  EEPROM.get(BARROW_EEPROP_OFFSET, barrow);
  if (barrow < 2800)
    barrow = 2992;

  altimeter.begin();

  InitCompass();

  lcd.begin(16, 2); // initialize the lcd for 20 chars 4 lines and turn on backlight
  lcd.backlight();
  //SetupLCDCustomCharacters();

//  encoder.write(barrow*4);

  pinMode(ALARM_PIN, OUTPUT);

  InitializeEncoder();

  CalibrationReset();
}

/*--(end setup )---*/

void loop()
{
  button.loop();
  int buttsUp = button.isReleased();

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

  page = (encoder/4 + PageCount) % PageCount;

  delay(10);
} /* --(end main loop )-- */
