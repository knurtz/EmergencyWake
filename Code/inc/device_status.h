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
        ew_time_t saved_time;           // as stored in eeprom
        ew_time_t modified_time;        // used while user sets up a new alarm time
        uint8_t snooze_timer;           // how many minutes to add to original alarm time due to snoozing user
        ew_alarmstate_t state;
} ew_alarm_t;


typedef struct {                        // Value after reset or standby:
    ew_state_t state;                   // set to EW_STARTUP
    ew_alarmnumber_t active_alarm;      // set to EW_ALARM_NONE

    ew_alarm_t alarms[2];               // saved_time restored from EEPROM, modified_time reset to 0:00, snooze_timer and state restored from RTC backup register

    ew_alarmnumber_t next_alarm;        // calculated by retrieveDeviceStatus() in main.c
    ew_time_t next_alarm_time;          

    uint8_t alarm_volume;               // restored from EEPROM
    uint8_t snooze_time;

    bool dst_enabled;                   // restored from RTC configuration backup bit
} ew_device_status_t;

#endif /* DEVICE_STATUS_H */
