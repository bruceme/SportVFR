#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal.h>
#include <EEPROM.h>
#include <SimpleKalmanFilter.h>

#include "platform/stm32f103.hpp"
#include "instruments/temperature.hpp"
#include "sensors.hpp"
#include "header.hpp"
#include "encoder.hpp"

// Display Variables
// RS, E, D4, D5, D6, D7
LiquidCrystal lcd(LCD_RS_PIN, LCD_EN_PIN, LCD_D4_PIN, LCD_D5_PIN, LCD_D6_PIN, LCD_D7_PIN);

/// @brief Display the current page and handle navigation between pages
/// @note Each page function handler can return DISP_OK to stay on the page, or DISP_RETURN to go back to the main page
void Display()
{
  // Refresh our sensors
  PollAllSensors();

  // if (page < 0 || page >= PageCount)
  //   page = Page0;

  // Do the page display navigation
  auto dispResult = PageDisplayLookup[page]();
  // if (DISP_RETURN == dispResult)
  // {
  //   page = Page0;
  // }
  // page += dispResult;
}

bool isButtonPressed();

/////////////////////// ---------- S  E  T  U  P ---------- ///////////////////////
void setup()
{
  //Serial.begin(9600);
  pinMode(ALARM_PIN, OUTPUT); 
  pinMode(BUTTON_PIN, INPUT_PULLUP);  // Use pull-up resistor
  analogReadResolution(12);  // 12 bit ADC

  // Remap I2C1 to PB8 (SCL) and PB9 (SDA)
  Wire.setSCL(PB8);
  Wire.setSDA(PB9);
  Wire.begin();

  EEPROM.get(BARROW_EEPROP_OFFSET, barrow);
  EEPROM.get(BARROW_CALIB_EEPROP_OFFSET, barrowOffset);
  if (barrow < 2800)
    barrow = 2992;

  setupEncoders();
  setupOAT();
  setupFuelFlowCapture();
  setupTachCapture();

  lcd.begin(16, 2); // initialize the lcd for 20 chars 4 lines and turn on backlight

  page = isButtonPressed() ? RawDisplayPage : EnginePage;
}

/////////////////////// ---------- L O O P  ---------- ///////////////////////
void loop()
{
  buttUp = buttonReleased();
  if (inError)
  {
    lcd.setCursor(0, 0);
    lcd.print(Error.c_str());
    inError = !buttUp;
    return;
  }

  Display();

  delay(10);
} /* --(end main loop )-- */
