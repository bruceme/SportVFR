#include <Arduino.h>
#include <SimpleKalmanFilter.h>
#include "header.hpp"

int page = Page0;
bool buttUp;
bool inError = false;
String Error = ""; // Error message
int blinker = 0;

MappedAirspeed airspeed(0); // pressure sensor
MappedADC oilPres(OP_PIN, 1, UNIT_PRESSURE);
MappedADC oilTemp(OT_PIN, 2, UNIT_TEMPERATURE);
MappedADC fuelPres(FP_PIN, 3, UNIT_PRESSURE);
FuelMap fuelQ1(FQ1_PIN, 4);
FuelMap fuelQ2(FQ2_PIN, 5);
int fuelFlow = 85;
MappedADC OAT(OAT_PIN,10); 
MappedADC AuxInput1(AUX1_PIN,6); 
MappedADC AuxInput2(AUX2_PIN,7); 
MappedADC AuxInput3(AUX3_PIN,8); 
MappedADC AuxInput4(AUX4_PIN,9); 

/// @brief Most of the true program-wide scoped globals
int heading = 360;
int altitude = 0;
char vsi = '^';
int tachometer = 0;
int lastmillis = millis();
byte revolutions = 0;
int altitudeLast;
bool powerOn = true;
int barrow;
int barrowOffest;
bool up;
bool down;
unsigned long now;
const int sensorCalibration = 3031 - 3034;         // current reference barrometeric pressure - pressure you read when you set your altitude
int barrowOffset = 2992 + sensorCalibration; // standard + offset for sensor calibration

// Use this to display raw sensor output for configuration
// #define SENSOR_CONFIG
int curDeltaTPhase = 0;