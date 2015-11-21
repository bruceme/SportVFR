
/*-----( Import needed libraries )-----*/
#include <Wire.h>  // Comes with Arduino IDE
// Get the LCD I2C Library here: 
#include <LiquidCrystal_I2C.h>

#include <SFE_BMP180.h>

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address

#include <EEPROM.h>

#define PARAM_OFFSET 4

struct _mappedPerm
{
  int slopeDen; // slope denominator
  int slopeNum; // slope numerator
  int yInt;
  int minIn;
  int maxIn;
};

class Mapped
{
private:
  _mappedPerm _permValues;

  byte ReadPin;
  byte FlashIndex;
  bool tooHigh = 0;
  bool tooLow = 0;

  int in0,in1,out0,out1;

  void get(){EEPROM.get(FlashIndex*sizeof(_mappedPerm) + PARAM_OFFSET, _permValues);}
  void put(){EEPROM.put(FlashIndex*sizeof(_mappedPerm) + PARAM_OFFSET, _permValues);}
    
public:
  
  Mapped(byte readPin, byte flashIndex)
  {
    ReadPin = readPin;
    FlashIndex = flashIndex;
    get();
  }

  void CalibrateLow(int outValue)
  {
      in0 = analogRead(ReadPin);
      out0 = outValue;
      Calibrate(in0, out0, in1, out1);
  }

  void CalibrateHigh(int outValue)
  {
      in1 = analogRead(ReadPin);
      out1 = outValue;
      Calibrate(in0, out0, in1, out1);
  }

  // If you ever have an 8th grader... show them this code, because you sometimes do _RARELY_ use slope-intercept in the real world
  void Calibrate(int in0, int out0, int in1, int out1)
  {
    _permValues.slopeDen = (in1 - in0);
    _permValues.slopeNum = (out1 - out0);
    _permValues.yInt = out0 - (int)((long)_permValues.slopeNum * (long)in0 / (long)_permValues.slopeDen);
    put();
  }

  
  int Read()
  {
    int inT = analogRead(ReadPin);
    tooLow = inT < _permValues.minIn;
    tooHigh = inT > _permValues.maxIn;
    return (int)((long)_permValues.slopeNum * (long)inT / (long)_permValues.slopeDen) + _permValues.yInt;
  }
};


#define HMC5883L_Address 0x1E //0011110b, I2C 7bit address of HMC5883

#define deltaAltVSI 10

#define TACH_PIN 2
const int tachIntNum = digitalPinToInterrupt(TACH_PIN);
#define BUTTON_UP_PIN 4
#define BUTTON_DOWN_PIN 5

#define BARROW_EEPROP_OFFSET 240


Mapped airspeed(A0,0);
Mapped oilPres(A1,1);
Mapped oilTemp(A2,2);
int heading = 360;
int altitude = 0;
char vsi = '^';
int tachometer = 0;
int lastmillis = millis();
byte revolutions = 0;
int altitudeLast;
int setBarrowModeCount = 5;
bool powerOn = true;
byte configModeIdx = 0;
int barrow;
bool up;
bool down;
unsigned long now;
SFE_BMP180 altimeter;

void setup()   
{
  Serial.begin(9800);
  attachTachInt();
  Wire.begin();
  
  Wire.beginTransmission(HMC5883L_Address); //open communication with HMC5883
  Wire.write(0x02); //select mode register
  Wire.write(0x00); //continuous measurement mode
  Wire.endTransmission();

  pinMode(BUTTON_UP_PIN, INPUT_PULLUP);
  pinMode(BUTTON_DOWN_PIN, INPUT_PULLUP);
  // pinMode(TACH_PIN, INPUT_PULLUP);

  EEPROM.get(BARROW_EEPROP_OFFSET, barrow);
  if (barrow < 2800)
    barrow = 2992;
  
  altimeter.begin();
      
  lcd.begin(16,2);         // initialize the lcd for 20 chars 4 lines and turn on backlight
}/*--(end setup )---*/

void loop()
{
  up = LOW == digitalRead(BUTTON_UP_PIN);
  down = LOW == digitalRead(BUTTON_DOWN_PIN);

  // Detect setup mode select on boot
  if (up && down && powerOn)
    configModeIdx = 1;
  powerOn = false;
  
  // fetch the altitude
  altitude = GetAltitude();
  
  if(configModeIdx > 0)
    ConfigureMode();
  else if (up || down || setBarrowModeCount < 0)
    SetPressure();
  else
    NormalAquireDisplay();
  
  delay(200); 
}/* --(end main loop )-- */

void DisplayValues()
{
  lcd.setCursor (0, 0);        // go to the next line
  lcd.print(Format(airspeed.Read(), 3, ' ')); //3
  lcd.print("   ");                           //6
  lcd.print(Format(heading - heading%5,       3, '0')); //9
  lcd.print((char)0xdf);                      //10
  lcd.print(Format(altitude - altitude%10,    5, ' ')); //15
  lcd.print(vsi);                             //16

  lcd.setCursor ( 0, 1 );        // go to the next line
  lcd.print (Format(oilPres.Read(), 3, ' ')); // 3
  lcd.print (Format(tachometer,    7, ' ')); // 10
  lcd.print (Format(oilTemp.Read(), 5, ' ')); // 15
  lcd.print ((char)0xdf);                       // 16
}

void DisplaySetPressure()
{
  lcd.setCursor (0, 0);           // go to the next line
  lcd.print("Set Pressure Alt"); 
  lcd.setCursor ( 0, 1 );         // go to the next line
  lcd.print(Format(barrow/100, 2, ' ')); // 2
  lcd.print('.');                        // 3
  lcd.print(Format(barrow%100, 2, '0')); // 5
  lcd.print(Format(altitude - altitude%10,    11, ' ')); //16
}

void NormalAquireDisplay()
{
  CleanupBarrow();

    int now = millis();
    if (now - lastmillis > 1000)
      tachometer = ReadTachometer();
    
  // -- heading --
    heading = ReadCompass();
   
    DisplayValues();
}


void ConfigureMode()
{
      lcd.setCursor (0, 0);
      switch(configModeIdx)
      {
        case 1:
          lcd.print("Setup Mode");
          lcd.setCursor (0, 1);
          lcd.print("^ Set - v Next");
          break;
        case 2:
          lcd.print("ASI - Static");
          if (up)
            airspeed.CalibrateLow(0);
          break;
        case 3:
          lcd.print("ASI - 6.5\" Water");
          if (up)
            airspeed.CalibrateHigh(100);
          break;
        case 4:
          lcd.print("Oil Temp - ICE");
          if (up)
            oilTemp.CalibrateHigh(0);
          break;
        case 5:
          lcd.print("Oil Temp - BOIL");
          if (up)
            oilTemp.CalibrateHigh(100);
          break;
        case 6:
          lcd.print("Oil Pres - Stat");
          if (up)
            oilPres.CalibrateLow(0);
        case 7:
          lcd.print("Oil Pres - Run Up");
          if (up)
            oilPres.CalibrateHigh(80);
          break;
        default:
          configModeIdx = 1;
          break;
      }
      
      if (down && !up)
      {
        lcd.clear();
        configModeIdx++;
      }
}

int GetAltitude()
{
  // -- altimeter --
  altitudeLast = altitude;

  int altT = (int)(altimeter.altitude(getPressure(), ((float)barrow)/2.952998) * 3.281);
  // -- vsi indicator --
  int deltaAlt = altitudeLast - altitude;
  vsi = deltaAlt > deltaAltVSI ?  '^' : deltaAlt < -deltaAltVSI ? 'v' : ' ';
  
  return altT;
}

void CleanupBarrow()
{
    if (setBarrowModeCount == 0)
    {
        lcd.clear();
        EEPROM.put(BARROW_EEPROP_OFFSET, barrow);
    }
    setBarrowModeCount = 5;
}

int ReadTachometer()
{
  detachInterrupt(tachIntNum); //Disable interrupt when calculating
  int tachT = revolutions * 60;
  revolutions = 0;
  attachTachInt();
  lastmillis = now;
  return tachT;
}

void SetPressure()
{
    if (setBarrowModeCount > 0)
    {
      setBarrowModeCount--;
    }
    else
    {
      if (up || down)
         setBarrowModeCount = -10;
      else 
         setBarrowModeCount++;

      if (up)
        barrow ++;
      if (down)
        barrow --;
  
      DisplaySetPressure();
    }
}

int ReadCompass()
{
  int x,y,z; //triple axis data

  //Tell the HMC5883 where to begin reading data
  Wire.beginTransmission(HMC5883L_Address);
  Wire.write(0x03); //select register 3, X MSB register
  Wire.endTransmission();
 
 //Read data from each axis, 2 registers per axis
  int err = Wire.requestFrom(HMC5883L_Address, 6);
  int avail = Wire.available();
  if(avail > 6){
    Serial.print('c');
    x = Wire.read()<<8; //X msb
    x |= Wire.read(); //X lsb
    z = Wire.read()<<8; //Z msb
    z |= Wire.read(); //Z lsb
    y = Wire.read()<<8; //Y msb
    y |= Wire.read(); //Y lsb
  }
  else
  {
    Serial.print('x');
  }


  return atan2deg(y, x);
}

int atan2deg(int y, int x)
{
    int maxT=max(y, x);
    if (maxT > 60)   // sqrt(2^31/3667)... overflows an internal value
    {
        x = 60 * x / maxT;
        y = 60 * y / maxT; 
    }

    if (y == 0)
        return (x >= 0) ? 360 : 180;

    if (x == 0)
        return (y > 0) ? 90 : 270;

    /* inner atan function */
    int aX = abs(x);
    int aY = abs(y);
    
    int angle = (aY <= aX) ? (3667 * aX * aY) / (64 * aX * aX + 17 * aY * aY) :
        90 - (3667 * aX * aY) / (64 * aY * aY + 17 * aX * aX);

    angle = (x < 0) ? 
        (y < 0) ? 180 + angle : 180 - angle :
        (y < 0) ? 360 - angle : angle;

    if (0 == angle)
        angle = 360;

    return angle;
}
#define INT_DIGITS 5

char *my_itoa(int i)
{
  /* Room for INT_DIGITS digits, - and '\0' */
  static char buf[INT_DIGITS + 2];
  char *p = buf + INT_DIGITS + 1;  /* points to terminating '\0' */
  if (i >= 0) {
    do {
      *--p = '0' + (i % 10);
      i /= 10;
    } while (i != 0);
    return p;
  }
  else {      /* i < 0 */
    do {
      *--p = '0' - (i % 10);
      i /= 10;
    } while (i != 0);
    *--p = '-';
  }
  return p;
}


char *Format(int value, int width, char pad)
{
  char *val = my_itoa(value);
  for(short i = width - strlen(val); i > 0; i--)
  {
    --val;
    val[0] = pad;
  }
  return val;
}


void tachInt(){
  revolutions++;
}

void attachTachInt(){
    attachInterrupt(tachIntNum, tachInt, FALLING);
} 

double getPressure()
{
  char status;
  double T,P,p0,a;

  // You must first get a temperature measurement to perform a pressure reading.
  
  // Start a temperature measurement:
  // If request is successful, the number of ms to wait is returned.
  // If request is unsuccessful, 0 is returned.

  status = altimeter.startTemperature();
  if (status != 0)
  {
    // Wait for the measurement to complete:

    delay(status);

    // Retrieve the completed temperature measurement:
    // Note that the measurement is stored in the variable T.
    // Use '&T' to provide the address of T to the function.
    // Function returns 1 if successful, 0 if failure.

    status = altimeter.getTemperature(T);
    if (status != 0)
    {
      // Start a pressure measurement:
      // The parameter is the oversampling setting, from 0 to 3 (highest res, longest wait).
      // If request is successful, the number of ms to wait is returned.
      // If request is unsuccessful, 0 is returned.

      status = altimeter.startPressure(3);
      if (status != 0)
      {
        // Wait for the measurement to complete:
        delay(status);

        // Retrieve the completed pressure measurement:
        // Note that the measurement is stored in the variable P.
        // Use '&P' to provide the address of P.
        // Note also that the function requires the previous temperature measurement (T).
        // (If temperature is stable, you can do one temperature measurement for a number of pressure measurements.)
        // Function returns 1 if successful, 0 if failure.

        status = altimeter.getPressure(P,T);
        if (status != 0)
        {
          return(P);
        }
      }
    }
  }
}

