#pragma once

#include <Arduino.h>
#include <SimpleKalmanFilter.h>
#include "platform/stm32f103.hpp"
#include "units.hpp"

#define Page0 0
#define RawDisplayPage 0
#define EnginePage     1
#define FuelPage       2
#define BaroPage       3
#define ConfigPage     4
#define PageCount 5

/// @brief Fuel tank calibration structure
/// Used to store calibration data for fuel quantity measurement
struct FuelCalibration {
    int tankSize;         // Total capacity of the tank (e.g., in liters or gallons)
    int stepInterval;     // Interval between calibration steps (e.g., every 5 liters)
    int currentStep;      // Current step index (0–7)

    int* range;           // Pointer to range values (e.g., sensor readings)
    int* domain;          // Pointer to domain values (e.g., actual fuel quantities)
};

// Sensor Variables
#define deltaAltVSI 100

#define BARROW_EEPROP_OFFSET 240
#define BARROW_CALIB_EEPROP_OFFSET 244

#define MIN_OilTemp 95
#define MAX_OilTemp 245
#define MIN_OilPress 25
#define MAX_OilPress 100
#define MIN_FuelPress 1
#define MAX_FuelPress 8
#define MIN_FuelQuantity 30 // 3.0g
#define ALARM_PIN PA12

#define ADC_MAX 4095

extern int page;
extern bool buttUp;
extern bool inError;
extern String Error; // Error message
extern int blinker;

#include "Mapped/MappedBase.hpp"
#include "Mapped/MappedADC.hpp"
#include "Mapped/MappedAirspeed.hpp"
#include "Mapped/FuelMap.hpp"


#define FeetPerMeter 3.28084
#define MPaPerBarrow_256 8669L // barrow is 100 * InHq this 256 bigger than it needs to be (simple math)


#define DISP_OK 0  // All good
#define DISP_RETURN 2 // Return to previous page

#include <LiquidCrystal.h>
extern LiquidCrystal lcd;

extern MappedAirspeed airspeed; // pressure sensor
extern MappedADC oilPres;
extern MappedADC oilTemp;
extern MappedADC fuelPres;
extern FuelMap fuelQ1;
extern FuelMap fuelQ2;
extern int fuelFlow;
extern MappedADC OAT; 
extern MappedADC AuxInput1; 
extern MappedADC AuxInput2; 
extern MappedADC AuxInput3; 
extern MappedADC AuxInput4; 

extern volatile int32_t encoder;
extern int heading;
extern int altitude;
extern char vsi;
extern int tachometer;
extern int lastmillis;
extern byte revolutions;
extern int altitudeLast;
extern bool powerOn;
extern int barrow;
extern int barrowOffset;
extern bool up;
extern bool down;
extern unsigned long now;
extern int curDeltaTPhase;

/// Display function prototypes
void PollAllSensors();
extern int (*PageDisplayLookup[])();

// Generic Headers
int GetAltitude();

void setupFuelFlowCapture();
void setupTachCapture();

void SetSensorConstants();
