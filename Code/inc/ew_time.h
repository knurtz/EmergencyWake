#ifndef EW_TIME_H
#define EW_TIME_H

#include "ch.h"
#include "hal.h"

typedef struct {
    uint8_t hours;
    uint8_t minutes;
    bool dst;               // only used for current time
    bool enabled;           // only used for alarms
} ew_time_t;

#endif /* EW_TIME_H */
