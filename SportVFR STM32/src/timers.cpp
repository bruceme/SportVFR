#include <Arduino.h>
#include "stm32f1xx_hal.h"
#include <stm32f1xx_hal_tim.h>
#include "header.hpp"

/// Fuel Flow and Tachometer timers and capture variables

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;

volatile uint32_t ff_last = 0;
volatile uint32_t ff_period = 0;

HardwareTimer *FlowTimer = nullptr;
void onFlowCapture()
{
    uint32_t now = FlowTimer->getCaptureCompare(1);
    ff_period = (now >= ff_last) ? (now - ff_last)
                                 : (0xFFFF - ff_last + now);
    ff_last = now;
}

void setupFuelFlowCapture()
{
    // TIM2 → Fuel flow on PA15 (CH1)
    FlowTimer = new HardwareTimer(TIM2);
    FlowTimer->setPrescaleFactor(72);   // 1 MHz
    FlowTimer->setOverflow(0xFFFF);
    FlowTimer->attachInterrupt(1, onFlowCapture);
    FlowTimer->resume();
}

uint32_t kFactorPPL = 450; // Pulses per liter for the fuel flow sensor

uint32_t GetFuelFlowDLPH()
{
    if (ff_period == 0) return 0;

    // (3,600,000,000 / (period * K)) except we need to do this in 32-bit math, so split the constant in half and double the answer
    return 2 * (1800000000L / (ff_period * kFactorPPL)); 
}


volatile uint32_t tach_last = 0;
volatile uint32_t tach_period = 0;
HardwareTimer *TachTimer = nullptr;

void onTachCapture()
{
    uint32_t now = TachTimer->getCaptureCompare(4);
    tach_period = (now >= tach_last) ? (now - tach_last)
                                     : (0xFFFF - tach_last + now);
    tach_last = now;
}

void setupTachCapture()
{
     // TIM1 → Tachometer on PA11 (CH4)
    TachTimer = new HardwareTimer(TIM1);
    TachTimer->setPrescaleFactor(72);   // 1 MHz
    TachTimer->setOverflow(0xFFFF);
    TachTimer->attachInterrupt(4, onTachCapture);
    TachTimer->resume();
}

int rpm_last;

int QuickReadTachometer()
{
    return rpm_last;
}

void SetTachPulsesPerRPM(int pulses)
{
    // This is a simple calculation based on the fact that RPM = (60,000,000 / (period in µs)) and period in µs = (pulses per revolution * time per pulse)
    // So if we want to adjust the RPM calculation based on a different number of pulses per revolution, we can just adjust the constant in the RPM calculation
    kFactorPPL = 450 * pulses / 2; // 450 is the default for 2-stroke engines, so we adjust it based on the number of pulses per revolution
}

int rpmHighWarning;
int rpmHighAlarm;

void SetTachAlarmRPMs(int highWarning, int highAlarm)
{
    rpmHighWarning = highWarning;
    rpmHighAlarm = highAlarm;
}

AlarmState ReadTachometer()
{
    if (tach_period == 0) return NONE;

    // Timer runs at 1 MHz → 1 µs per tick
    rpm_last = 60000000L / tach_period; // 60,000,000 µs per minute divided by period in µs per revolution gives us RPM

    if (rpm_last >= rpmHighAlarm)
        return HIGH_ALARM;
    else if (rpm_last >= rpmHighWarning)
        return HIGH_WARNING;
    else
        return NONE;
}