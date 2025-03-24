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
