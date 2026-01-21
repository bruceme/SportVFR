#pragma once

#ifdef QMC5883L

#include <QMC5883L.h>
#include <Wire.h>
#include <SimpleKalmanFilter.h>
QMC5883L compass;

SimpleKalmanFilter compassxFilter(2, 2, 0.01);
SimpleKalmanFilter compasszFilter(2, 2, 0.01);

void InitCompass()
{
  compass.init();
  compass.setSamplingRate(50);
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

  auto alpha = (int)(atan2(x, z) * 180.0 / PI);
  alpha = (int)((alpha < 0) ? (alpha + 360.0) : alpha); 

  sensor_t sensor;
  mag.getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" uT");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" uT");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" uT");  
  Serial.print  ("X: "); Serial.println(x);
  Serial.print  ("Y: "); Serial.println(y);
  Serial.print  ("Z: "); Serial.println(z);
  Serial.print  ("A: "); Serial.println(alpha);
  Serial.println("------------------------------------");
  Serial.println("");


  delay(500);

  lcd.print(x);
  lcd.print(" ");
  lcd.print(y);
  lcd.setCursor(1,1);
  lcd.print(z);
  lcd.print(" ");
  lcd.setCursor(8,1);
  lcd.print(alpha);
}

// 0   N - 0, -14
// 90  E - 15,0
// 180 S - 0, 40
// 270 W - -26, 0
#endif

#endif