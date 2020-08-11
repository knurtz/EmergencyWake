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

typedef enum {
    EW_ALARM_ONE,
    EW_ALARM_TWO,
    EW_ALARM_NONE
} ew_alarmnumber_t;

typedef struct {
    uint8_t hours;
    uint8_t minutes;
} ew_time_t;

typedef struct {
        ew_time_t saved_time;           // alarm as it is set up by the user
        ew_time_t modified_time;        // used while the user sets up a new alarm time (before saving)
        uint8_t snooze_timer;           // how many minutes to add to original alarm time due to a snoozing user
        ew_alarmstate_t state;          // alarm state
} ew_alarm_t;

typedef struct {
    ew_state_t state;                   // current state determines what's shown on screen
    ew_alarmnumber_t active_alarm;      // which alarm is currently ringing

    ew_alarm_t alarms[2];               // two user definable alarms

    ew_alarmnumber_t next_alarm;        // which alarm is going to fire next (taking snoozing into consideration). Value is calculated by findNextAlarm().
    ew_time_t next_alarm_time;          // time at which the next alarm fires. Value is calculated by findNextAlarmTime().

    uint8_t alarm_volume;               // user settings
    uint8_t snooze_time;

    bool dst_enabled;                   // daylight savings time
} ew_device_status_t;


extern ew_device_status_t device_status;    // global variable defined in main.c


#endif /* DEVICE_STATUS_H */
