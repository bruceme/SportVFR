#include <Arduino.h>
#include "header.hpp"

#include "instruments/altimeter.hpp"
//#include "instruments/compass.hpp"
#include "utility.hpp"
//#include "encoder.hpp"
#include "sensors.hpp"

void DisplayOneDec(int value)
{
    lcd.print(Format(value / 10, 2, ' ')); // 0 fuelQ1
    lcd.print('.');
    lcd.print((char)('0' + (value % 10))); // 0 fuelQ1
}

int AirspeedHeadingAltitude()
{
    lcd.setCursor(0, 1);                             // set cursor

    // lcd.print(Format(airspeed.QuickRead(), 3, ' ')); // 0 airspeed
    // lcd.print("   ");                                 // 3
    // lcd.print(Format(heading, 3, ' '));              // 7
    // lcd.print((char)0xdf);                           // 8 degree symbol
    // lcd.print(' ');                                  // 9
    // lcd.print(Format(altitude, 5, ' '));             // 16

    DisplayOneDec(FuelQuantity1.lastValue);
    lcd.print ("g ");
    DisplayOneDec(FuelFlow.lastValue);
    lcd.print (' ');
    DisplayOneDec(FuelQuantity2.lastValue);
    lcd.print ('g');

    return DISP_OK;
}

const char *to_hex12(uint32_t value);

int DisplayRawValuesPage()
{
    lcd.setCursor(0, 0);
    lcd.print(to_hex12(analogRead(FuelQuantity1.pin))); // 0 fuelQ1
    lcd.print("-Raw-");                 // 4
    lcd.print(to_hex12(analogRead(FuelQuantity2.pin))); // 11 fuelQ2

    lcd.setCursor(0, 1);
    lcd.print("    ");
    lcd.print(to_hex12(analogRead(OilTemperature.pin)));
    lcd.print(" ");
    lcd.print(to_hex12(analogRead(OilPressure.pin)));
    lcd.print(" ");
    lcd.print(to_hex12(analogRead(FuelPressure.pin)));
    delay(500);
    return DISP_OK;
}

int DisplayEnginePage() {
    AirspeedHeadingAltitude();

    lcd.setCursor(0, 0);                             // set cursor
    lcd.print(Format(Tachometer.lastValue, 4, '0'));           // 4
    lcd.print(' ');                                  // 5
    lcd.print(Format(OilTemperature.lastValue, 3, ' '));  // 8
    lcd.print((char)0xdf);                           // 9 deg
    lcd.print(Format(OilPressure.lastValue, 4, ' '));  // 13
    lcd.print(Format(FuelPressure.lastValue, 3, ' ')); // 16

    return DISP_OK;
}

void DisplayOneDecimalInteger(int value);

int DisplayFuelPage() {
    AirspeedHeadingAltitude();

    DisplayOneDecimalInteger(fuelQ1.Read());    
    lcd.print('g');                      // 5
    lcd.print("<fuel>");                 // 11
    DisplayOneDecimalInteger(fuelQ2.Read());    
    lcd.print('g');                      // 16

    lcd.print(Format(oilTemp.QuickRead(), 3, ' '));  // 8
    lcd.print((char)0xdf);                           // 9 deg
    lcd.print(Format(oilPres.QuickRead(), 4, ' '));  // 13
    lcd.print(Format(fuelPres.QuickRead(), 3, ' ')); // 16

    return DISP_OK;   
  }

void DisplayOneDecimalInteger(int value)
{
    lcd.print(value / 10); // 2
    lcd.print('.');          // 3
    lcd.print(value % 10); // 4
}


void DisplayBarro(String Messsage) { 
    lcd.setCursor(0, 0); // go to the next line
    lcd.print(Messsage);
    lcd.setCursor(0, 1);                     // go to the next line
    lcd.print(Format(barrow / 100, 2, ' ')); // 2
    lcd.print('.');                          // 3
    lcd.print(Format(barrow % 100, 2, '0')); // 5
    lcd.print("      ");
    lcd.print(Format(altitude, 5, ' ')); // 16
 }

int DisplayBaroPage() { 
  barrow = encoder / 2;
  DisplayBarro("Set Pressure Alt");

  if (buttUp)
  {
    lcd.clear();
    EEPROM.put(BARROW_EEPROP_OFFSET, barrow);
    return DISP_RETURN;
  }

  altitude = GetAltitude();

  return DISP_OK;
}

FuelCalibration fuelCal = {
    .tankSize = 16,
    .stepInterval = 16/8,
    .currentStep = 0,
    .range = nullptr,
    .domain = nullptr};


struct PromptHandler {
    byte menuId;                 // Menu identifier
    const char* prompt;                 // Prompt to display (e.g., "Enter fuel level:")
    void (*onPageFlip)(int newValue);    // Function pointer to set the value
    void (*displayValue)(int currentValue) = nullptr; // Optional function pointer to display the current value
    int (*initEncoderValue)() = nullptr; // Optional function pointer to initialize the encoder value
};

String calibrationMenu[] = {
    "Calibration Menu",
    "OAT & Oil Temp",
    "Fuel Pressure",
    "Airspeed Sensor",
    "Oil Pressure",
    "Compass",
    "Tank 1",
    "Tank 2",
    "Fuel Totalizer",
    "Altimeter"
};

int fuelPressureMax = 8;// or 60psi
int pulsesPerGallon = 1640; // default value

/// @brief Handlers for prompting and configuration menu
/// These are typically actions for calibrations or messages to enter

// Define the array with inline lambdas or static functions
PromptHandler calibrationHandlers[] = {
    {0, "Use Enc Button", [](int v) { /* No action needed for menu title */ } },
    // Temp Sensors
    {1, "Apply 0C", [](int v) { OAT.CalibrateLow(0); } },
    {1, "Apply 100C",   [](int v) { OAT.CalibrateHigh(100.0); } },
    
    // Fuel Pressure Low-Pressure Sensors
    {2, "Low/High",   [](int v) { fuelPressureMax = v==0?8:60; }, [](int v)
        { 
            lcd.print((v ==0)?"8psi Max":"60psi Max");
        }},
    {2, "Apply 0-psi",   [](int v) { fuelPres.CalibrateLow(0); } },
    {2, "Apply ",  [](int v) { fuelPres.CalibrateHigh(fuelPressureMax); }, [](int v)
        {
            lcd.print(fuelPressureMax);
            lcd.print("-psi");
        }},
    // Airspeed Sensor
    {3, "Apply 0mph",  [](int v) { airspeed.CalibrateLow(0); } },
    {3, "Apply 60mph",  [](int v) { airspeed.CalibrateHigh(52.14); } }, // 52.1 kts is 60mph

    // Oil Pressure 
    {4, "Apply 0-psi",   [](int v) { oilPres.CalibrateLow(0); } },
    {4, "Apply 30-psi",  [](int v) { oilPres.CalibrateHigh(30.0);} },

    // Compass
    {5, "Swing compass...", [](int v) { /* compass.Calibrate(); */ } },

    // Fuel
    {6, "Total Qty: ",  [](int v) { fuelCal.tankSize = v; }, DisplayOneDecimalInteger, []{return 160;}},
    {6, "Fill To: ",  [](int v) { fuelCal.currentStep = v; }, DisplayOneDecimalInteger, []{return fuelCal.currentStep;}},
    {7, "Total Qty: ",  [](int v) { fuelCal.tankSize = v; } , DisplayOneDecimalInteger, []{return 160;}},
    {7, "Fill To: ",  [](int v) { fuelCal.currentStep = v; }, DisplayOneDecimalInteger , []{return fuelCal.currentStep;}},
    {8, "P/Gal: ",  [](int v) { pulsesPerGallon = v; }, nullptr, []{return pulsesPerGallon; } },
  
    // Altimeter   
    {9, "Set Current Alt: ",  [](int v) { barrowOffset = v;}, [](int v) { lcd.print(GetAltitude()); }, []{return barrowOffset; } }, 
};


const int CalibrationPageMax = sizeof(calibrationHandlers) / sizeof(calibrationHandlers[0]);

//  Calibration Modes -
//  Sensors that can be calibrated:
// Air Sensors
//    Airspeed  - Calibrate 0 and 60mph
//    Altimeter - Calibrate to known altitude reference
//    Compass   - Swing in a circle running a calibration routine
//    OAT       - Calibrate 0C and 100C
// Engine Sensors
//    Oil Temp  - Calibrate 0C and 100C
//    Oil Pressure -    Calibrate 0 and 30psi
//    Fuel Pressure-    Calibrate 0 and 8psi (or 30psi)
//    Fuel Quantity Tanks- Ask for tank size, then calibrate 8 steps from empty to full for each tank
//    Fuel totalizer-   pulses per gallon / updated on fill-up
//    Aux Inputs-       Calibrate at known values prompting user for requested display values

int curCalibrationPage = 0;
int encoderLast = 0;

int PromptItem(PromptHandler* promptItem, String menuItem);


int DisplayMenu(PromptHandler *menuItems, int itemCount)
{
    auto curMenuId = menuItems[curCalibrationPage].menuId;
    auto menuItemLabel = calibrationMenu[curMenuId];

    lcd.setCursor(0, 0);                           // go to the next line
    lcd.print(calibrationMenu[curMenuId]); // 16
 
    lcd.setCursor(0, 1);                           // go to the next line
    auto curPrompt = menuItems[curCalibrationPage].prompt;
    lcd.print(curPrompt);                // 16

    while (encoder != encoderLast && 
            curCalibrationPage < itemCount && 
            menuItems[curCalibrationPage].menuId == curMenuId)
        curCalibrationPage=(encoder > encoderLast)? curCalibrationPage+1: ((encoder > 0) ? curCalibrationPage : itemCount)-1;
        
    encoderLast = encoder;
        
    if (buttUp)
    {
        for(int i = 20; i > 0 || PromptItem(&menuItems[curCalibrationPage], menuItemLabel) != DISP_RETURN; i--)
            ;

        lcd.clear();
        return DISP_OK;
    }

    return DISP_OK;
}

int PromptItem(PromptHandler* promptItem, String menuItemLabel)
{
    lcd.setCursor(0, 0);                           // go to the next line
    lcd.print(menuItemLabel); // 16
    lcd.setCursor(0, 1);                           // go to the next line
    lcd.print(promptItem->prompt);                // 16
    if (promptItem->displayValue != nullptr)
    {
        promptItem->displayValue(encoder);
    }

    if (buttUp)
    {
        lcd.clear();
        
        if (promptItem->onPageFlip != nullptr)
            promptItem->onPageFlip(encoder);
  
        return DISP_RETURN;
    }

    return DISP_OK;
}

/// @brief Go through all the prompts and show the user what they can calibrate next
/// @return 
int CalibrationMenu()
{
    DisplayMenu(calibrationHandlers, CalibrationPageMax);

    return DISP_OK;
}


//  Configuration Modes
//  Sensors that can be configured:
//   Units (Metric/Imperial) for Distance, Temp, Pressure
//   4-stroke/2-stroke mode
//   Disable Instruments (Fuel Gauges, Oil Temp, Oil Press, Airspeed, Compass, Altimeter)
//   Tachometer pulses per RPM 
//   Aux Inputs, Set Name, Units and Alarms

String configMenu[] = {
    "Configuration Menu",
    "Stroke Type",
    "Units of Measure",
    "Fuel Gauges",
    "Oil Temp",
    "Oil Press",
    "Airspeed",
    "Compass",
    "Altimeter",
    "Tach Pulses/RPM"
};

#define UNIT_OF_VELOCITY_COUNT 3
String  UnitsOfVelocity[UNIT_OF_VELOCITY_COUNT] = {
    "KTS",
    "MPH",
    "KPH"
};

byte unitVel = VELOCITY_KNOTS;
byte unitDist = 0; // 0=Meters, 1=Feet
byte unitTemp = 0; // 0=Celsius, 1=Fahrenheit       
byte unitPressure = 0; // 0=hPa/mBar, 1=Psi/InHq
byte unitVolume = 0; // 0=Liters, 1=Gallons
bool isFourStroke = true;
int pulsesPerRPM = 2; // Tach pulses per RPM

#define UNIT_VELOCITY_EEPROP_OFFSET 	0
#define UNIT_DISTANCE_EEPROP_OFFSET 	1
#define UNIT_TEMPERATURE_EEPROP_OFFSET 	2
#define UNIT_PRESSURE_EEPROP_OFFSET		3
#define UNIT_VOLUME_EEPROP_OFFSET		4
#define FOUR_STROKE_EEPROP_OFFSET		5
#define TACH_PULSES_RPM_EEPROP_OFFSET	6


/// @brief Handlers for prompting and configuration menu
/// These are typically actions for calibrations or messages to enter
PromptHandler configPrompts[] = {
    {0, "Use Enc Button", [](int v) { /* No action needed for menu title */ } },
    {1, "Calibration", [](int v){/* start the calibration menu here (nested) */ }},
    {2, "Velocity",   [](int v) { unitVel = v % UNIT_OF_VELOCITY_COUNT; }, [](int v)
        {
            lcd.print(UnitsOfVelocity[v % UNIT_OF_VELOCITY_COUNT]);
        }},
    {2, "Distance",   [](int v) { unitDist = v%2; }, [](int v)
        { 
            lcd.print((v ==0)?"Meters":"Feet");
        }},
    {2, "Temperature",   [](int v) { unitTemp = v%2; }, [](int v)
        { 
            lcd.print((v ==0)?"Celsius":"Fahrenheit");
        }},
    {2, "Pressure",   [](int v) { unitPressure = v%2; }, [](int v)
        { 
            lcd.print((v ==0)?"hPa/mBar":"Psi/InHq");
        }},
    {2, "Volume",   [](int v) { unitVolume = v%2; }, [](int v)
        { 
            lcd.print((v ==0)?"Liters":"Gallons");
        }},
    {3, "2-Stroke/4-Stroke",   [](int v) { isFourStroke = v%2; }, [](int v)
        { 
            lcd.print((v ==0)?"2-Stroke":"4-Stroke");
        }},
    // {3, "Airspeed",   [](int v) { airspeed.Disable; }, [](int v)
    //     { 
    //         lcd.print(airspeed.Disabled ?"Disabled":"Enabled");
    //     }},
    // {3, "Compass",   [](int v) { compass.Disable; }, [](int v)
    //     { 
    //         lcd.print(compass.Disabled?"Disabled":"Enabled");
    //     }},
    // {3, "Altimeter",   [](int v) { altimeter.Disable; }, [](int v)
    //     { 
    //         lcd.print(altimeter.Disabled?"Disabled":"Enabled");
    //     }},
    {4, "Tank 1",   [](int v) { fuelQ1.Disabled; }, [](int v)
        { 
            lcd.print(fuelQ1.Disabled?"Disabled":"Enabled");
        }},
    {4, "Tank 2",   [](int v) { fuelQ2.Disabled; }, [](int v)
        { 
            lcd.print(fuelQ2.Disabled?"Disabled":"Enabled");
        }},
    {4, "Oil Temp",   [](int v) { oilTemp.Disabled; }, [](int v)
        { 
            lcd.print(oilTemp.Disabled?"Disabled":"Enabled");
        }},
    {4, "Oil Press",   [](int v) { oilPres.Disabled; }, [](int v)
        { 
            lcd.print(oilPres.Disabled?"Disabled":"Enabled");
        }},
    {4, "Fuel Pressure",   [](int v) { fuelPres.Disabled; }, [](int v)
        { 
            lcd.print(fuelPres.Disabled?"Disabled":"Enabled");
        }},

    {5, "Tach Pulses/RPM",  [](int v) { pulsesPerRPM = v; }, nullptr, []{return pulsesPerRPM; } },
    {6, "Save and Exit", [](int v) { 
        EEPROM.put(UNIT_VELOCITY_EEPROP_OFFSET, unitVel);
        EEPROM.put(UNIT_DISTANCE_EEPROP_OFFSET, unitDist);
        EEPROM.put(UNIT_TEMPERATURE_EEPROP_OFFSET, unitTemp);
        EEPROM.put(UNIT_PRESSURE_EEPROP_OFFSET, unitPressure);
        EEPROM.put(UNIT_VOLUME_EEPROP_OFFSET, unitVolume);
        EEPROM.put(FOUR_STROKE_EEPROP_OFFSET, isFourStroke);
        EEPROM.put(TACH_PULSES_RPM_EEPROP_OFFSET, pulsesPerRPM);
    } }
};

int (*PageDisplayLookup[])() = { DisplayRawValuesPage, DisplayEnginePage, DisplayFuelPage, DisplayBaroPage, [](){
    return DisplayMenu(configPrompts, sizeof(configPrompts) / sizeof(PromptHandler));
} };
