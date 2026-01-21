#pragma once

enum UnitType {
    UNIT_UNITLESS,    // e.g., counts, raw values
    UNIT_DISTANCE,    // e.g., meters, feet
    UNIT_VELOCITY,    // e.g., knots, mph
    UNIT_PRESSURE,    // e.g., psi, bar
    UNIT_TEMPERATURE,  // e.g., °C, °F
    UNIT_VOLUME       // e.g., liters, gallons
};

enum UnitsOfVelocity
{
    VELOCITY_KNOTS,
    VELOCITY_MPH,
    VELOCITY_KPH
};

enum UnitsOfPressure
{
    PRESSURE_PSI,
    PRESSURE_BAR,
    PRESSURE_INHG
};