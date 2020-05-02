#ifndef DEVICE_STATUS_H
#define DEVICE_STATUS_H

#include "ch.h"

typedef enum {
    EW_STARTUP,
    EW_INIT,
    EW_IDLE,
    EW_SHOW_ALARM,
    EW_SET_HOURS,
    EW_SET_MINUTES,
    EW_ALARM_RINGING,
    EW_STANDBY,
    EW_ERROR,
} ew_state_t;

typedef enum {
    EW_ALARM_DISABLED,
    EW_ALARM_ENABLED,
    EW_ALARM_SNOOZING
} ew_alarmstate_t;

typedef struct {
    uint8_t hours;
    uint8_t minutes;
} ew_time_t;

typedef struct {
        ew_time_t saved_time;           // as stored in eeprom
        ew_time_t modified_time;        // used while user sets up a new alarm time
        uint8_t snooze_timer;           // how many minutes to add to original alarm time due to snoozing user
        ew_alarmstate_t state;
} ew_alarm_t;


typedef struct {
    ew_state_t state;

    ew_alarm_t alarms[2];

    uint8_t next_alarm;                 // 0 - both disabled, 1 - alarm one, 2 - alarm two
    ew_time_t next_alarm_time;          // this is the time that gets registered in RTC alarm B module

    uint8_t alarm_volume;               // for now: actual value sent to audio IC - later: value between 0 and 100 percent -> figure out mapping
    uint8_t snooze_time;                // snooze time in minutes

    bool dst_enabled;
} ew_device_status_t;

#endif /* DEVICE_STATUS_H */
