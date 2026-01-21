#pragma once

typedef enum {
    NONE = 0,
    HIGH_ALARM,
    HIGH_WARNING,
    LOW_ALARM,
    LOW_WARNING,
    BAD_DATA_ALARM,
    BAD_STATE_ALARM,
    DATA_MISSING_ALARM
} AlarmState;