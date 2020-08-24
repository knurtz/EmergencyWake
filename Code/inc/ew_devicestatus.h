#ifndef DEVICE_STATUS_H
#define DEVICE_STATUS_H

#include "ch.h"
#include "ew_time.h"

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

typedef struct {
    ew_state_t state;                   // current device state and ...
    ew_alarmnumber_t toggle_state;      // ... state of toggle switch determine what is shown on screen
    ew_alarmnumber_t active_alarm;      // which alarm is currently ringing

    ew_alarm_t alarms[2];               // two user definable alarms

    ew_alarmnumber_t next_alarm;        // which alarm is going to fire next (taking snoozing into consideration). Value is calculated by findNextAlarm().
    ew_time_t next_alarm_time;          // time at which the next alarm fires. Value is calculated by findNextAlarmTime().

    uint8_t alarm_volume;               // user settings
    uint8_t snooze_time;
    ew_time_t modified_time;            // used while user sets up a new time
    bool dst_enabled;                   // daylight savings time

    bool unsaved_changes;               // unsaved changes that needs to be saved to eeprom before going into deep sleep 
                                        // (i.e. alarm times and state, snooze time and volume)
} ew_device_status_t;


extern ew_device_status_t device_status;    // global variable defined in device_status.c


ew_alarmnumber_t findNextAlarm(void);
ew_time_t findNextAlarmTime(void);
bool isAlarmEnabled(ew_alarmnumber_t alarm);
void updateToggleState(void);
void retrieveDeviceStatus(void);


#endif /* DEVICE_STATUS_H */
