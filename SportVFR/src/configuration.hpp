bool inConfigMode = false; 
bool inSetBarrowMode = false;
int configPage = 1;

void WaitRelease();

byte skipCount = 20;
void SetPressureDisplay(bool buttsUp)
{

}

void WaitRelease()
{
    for (int i = 0; i < 100; i++)
    {
        delay(10);
        button.loop();
        if (!button.isReleased())
            break;
    }
}

void CalibrationReset()
{
  airspeed.CalibrateLow(1);
  airspeed.CalibrateHigh(60,100);
  oilPres.CalibrateLow(0);
  oilPres.CalibrateHigh(90, 100);
  oilTemp.CalibrateLow(70);
  oilTemp.CalibrateHigh(212, 100);
  fuelPres.CalibrateLow(0);
  fuelPres.CalibrateHigh(30, 100);   
  fuelQ1.CalibrateLow(0);
  fuelQ1.CalibrateHigh(160, 100);
  fuelQ2.CalibrateLow(0);
  fuelQ2.CalibrateHigh(160,100);
}

const char modes[] = "ASCHFPOPOTTTF1F2";

void ConfigureModeDisplay(bool buttsUp)
{
    lcd.clear();
    lcd.setCursor(0, 0);
    switch (configPage)
    {
    case 1:
        lcd.print("Setup Mode");
        lcd.setCursor(0, 1);
        lcd.print("Press to Exit");
        if (buttsUp)
            inConfigMode = false;
        break;
    case 2:
        lcd.print("ASI - Static");
        if (buttsUp)
            airspeed.CalibrateLow(0);
        break;
    case 3:
        lcd.print("ASI - @60");
        if (buttsUp)
            airspeed.CalibrateHigh(60);
        break;
    case 4:
        lcd.print("Oil Temp - ICE");
        if (buttsUp)
            oilTemp.CalibrateHigh(0);
        break;
    case 5:
        lcd.print("Oil Temp - BOIL");
        if (buttsUp)
            oilTemp.CalibrateHigh(212);
        break;
    case 6:
        lcd.print("Oil Pres - Stat");
        if (buttsUp)
            oilPres.CalibrateLow(0);
    case 7:
        lcd.print("Oil Pres - Run Up");
        if (buttsUp)
            oilPres.CalibrateHigh(80);
        break;
    case 8:
        lcd.print("Reset All");
        if (buttsUp)
            CalibrationReset();
        break;        
    default:
        configPage = 1;
        break;
    }

//    configPage = encoder.read();
    if (configPage < 0)
         configPage = 8;
    if (configPage > 8)
         configPage = 1;
}