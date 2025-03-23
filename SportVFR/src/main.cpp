
/*-----( Import needed libraries )-----*/
// Get the LCD I2C Library here:
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4); // set the LCD address to 0x27 for a 16 chars and 2 line display

#include <ezButton.h>
ezButton button(6); // 6 on the PCB

#include <EEPROM.h>
#include <SimpleKalmanFilter.h>

#define PARAM_OFFSET 4

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

#include "mapped.hpp"
Mapped airspeed(A8, 0); // pressure sensor
Mapped oilPres(A1, 1);
Mapped oilTemp(A0, 2);
Mapped fuelPres(A2, 3);
Mapped fuelQ1(10,4);
Mapped fuelQ2(9,5);
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
const int barrowOffset = 3019 - 2995 - 7;  // current local barrow setting - what SportVFR shows as your altitude

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

void NormalDisplay()
{
  now = millis();
  if (now - lastmillis > 500)
    tachometer = ReadTachometer();

  // -- heading --
  heading = ReadCompass();


  auto ot = oilTemp.Read();
  auto op = oilPres.Read();
  auto fp = fuelPres.Read();
  auto fq1 = fuelQ1.Read(); fq1 = fq1 < 0 ? 0 : fq1;
  auto fq2 = fuelQ2.Read(); fq2 = fq2 < 0 ? 0 : fq2;

  auto alarmT = (ot > MAX_OilTemp || 
      ot < MIN_OilTemp ||
      op > MAX_OilPress ||
      op < MIN_OilPress ||
      fp > MAX_FuelPress ||
      fp < MIN_FuelPress ||
      fq1 < MIN_FuelQuantity ||
      fq2 < MIN_FuelQuantity 
    ) && blinker > 10;

  blinker = (blinker + 1 ) % 20;
  digitalWrite(ALARM_PIN, alarmT ? HIGH : LOW);

//  lcd.clear();

  lcd.setCursor(0, 0);                                 // go to the next line
  lcd.print(Format(airspeed.Read(), 3, ' '));          // 3
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
    lcd.print(Format(oilTemp.Read(), 3, ' ')); // 8
    lcd.print((char)0xdf);                     // 9 deg
    lcd.print(Format(oilPres.Read(), 4, ' ')); // 13
    lcd.print(Format(fuelPres.Read(), 3, ' ')); // 16
  }
  else if (page == FuelPage)
  {
    lcd.print(Format(fq1/10, 2, ' '));     // 2
    lcd.print('.');                           // 3
    lcd.print(Format(fq1%10, 1, '0'));     // 4
    lcd.print('g');                           // 5
    lcd.print("-fuel-");                       // 11
    lcd.print(Format(fq2/10, 2, ' '));     // 13
    lcd.print('.');                           // 14
    lcd.print(Format(fq2%10, 1, '0'));     // 15
//    lcd.print('g');                           // 16

    lcd.print(Format(oilTemp.Read(), 3, ' ')); // 8
    lcd.print((char)0xdf);                     // 9 deg
    lcd.print(Format(oilPres.Read(), 4, ' ')); // 13
    lcd.print(Format(fuelPres.Read(), 3, ' ')); // 16
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

  // fetch the altitude
  altitude = GetAltitude();

  if (inConfigMode)
    ConfigureModeDisplay(buttsUp);
  else if (!inSetBarrowMode)
    NormalDisplay();

  page = (encoder/4 + PageCount) % PageCount;

  delay(10);
} /* --(end main loop )-- */
