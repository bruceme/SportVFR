#include <Adafruit_HMC5883_U.h>
Adafruit_HMC5883_Unified mag = Adafruit_HMC5883_Unified(12345);

SimpleKalmanFilter compassxFilter(2, 2, 0.01);
SimpleKalmanFilter compasszFilter(2, 2, 0.01);

void InitCompass()
{
  for(int i = 0; i < 10 &&!mag.begin() ; i++)
  {
    /* There was a problem detecting the HMC5883 ... check your connections */
//    Serial.println("Ooops, no HMC5883 detected ... Check your wiring!");
    delay(10);
  }    
}

int ReadCompass()
{
  sensors_event_t event;
  mag.getEvent(&event);

  auto alpha = atan2(compassxFilter.updateEstimate(event.magnetic.x), 
    compasszFilter.updateEstimate(event.magnetic.z)) * 180.0 / PI;
  return (int)((alpha < 0) ? (alpha + 360.0) : alpha); 
//  return compassFilter.updateEstimate(atan2deg(100*event.magnetic.z, 100*event.magnetic.x));
}

//#define SHOW_RAW_COMPASS

#ifdef SHOW_RAW_COMPASS
SimpleKalmanFilter compassyFilter(2, 2, 0.01);
int ShowRawCompass()
{
  sensors_event_t event;
  lcd.setCursor(2,0);
  mag.getEvent(&event);
  auto x = compassxFilter.updateEstimate(event.magnetic.x);
  auto y = compassyFilter.updateEstimate(event.magnetic.y);
  auto z = compasszFilter.updateEstimate(event.magnetic.z);
  lcd.print(x);
  lcd.print(" ");
  lcd.print(y);
  lcd.setCursor(1,1);
  lcd.print(z);
  lcd.print(" ");
  lcd.setCursor(8,1);
  auto alpha = (int)(atan2(x, z) * 180.0 / PI);
  if (alpha <= 180)
    lcd.print(Format((alpha <= 0) ? (alpha + 360) : alpha, 3, '0'));
}

// 0   N - 0, -14
// 90  E - 15,0
// 180 S - 0, 40
// 270 W - -26, 0

#endif