#include <Adafruit_HMC5883_U.h>
Adafruit_HMC5883_Unified mag = Adafruit_HMC5883_Unified(12345);

SimpleKalmanFilter compassFilter(2, 2, 0.01);

int ReadCompass()
{
  sensors_event_t event;
  mag.getEvent(&event);
  return compassFilter.updateEstimate(atan2deg(event.magnetic.z, event.magnetic.x));
}